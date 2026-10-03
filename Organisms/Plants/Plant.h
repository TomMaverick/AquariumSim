#ifndef AQUARIUMSIM_PLANT_H
#define AQUARIUMSIM_PLANT_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "../../Simulation/WaterParameters.h"

class Plant {
public:
    std::string common_name, scientific_name;
    double biomass_g, health_hp = 100.0, max_health_hp = 100.0, age_days = 0.0;
    bool is_floating = false;

    std::map<std::string, double> stress_timers;
    std::vector<std::string> display_stresses;

    double current_height_cm = 5.0, max_submersed_height_cm = 30.0, max_emersed_height_cm = 10.0;
    bool can_grow_emersed = true;
    double base_growth_rate_cm_per_day = 1.0, coloration_factor = 0.0, density_factor = 1.0;

    double nitrogen_consumption_mg_per_hour = 0.0, po4_consumption_mg_per_hour = 0.0;
    double fe_consumption_mg_per_hour = 0.0, k_consumption_mg_per_hour = 0.005;
    double co2_consumption_mg_per_hour = 0.0, o2_production_mg_per_hour = 0.0;

    double min_temp_c = 20.0, max_temp_c = 28.0, min_ph = 6.0, max_ph = 8.0, min_gh = 2.0, max_gh = 20.0;

    virtual ~Plant() = default;

    void trackStress(const std::string& name, bool condition, double deltaTime, double threshold_hours) {
        if (condition) stress_timers[name] += deltaTime;
        else stress_timers[name] = std::max(0.0, stress_timers[name] - deltaTime * 3.0);
        if (stress_timers[name] > threshold_hours) display_stresses.push_back(name);
    }

    virtual void updateGrowth(const WaterParameters& water, double active_light, bool is_day, double deltaTime_hours) {
        age_days += deltaTime_hours / 24.0;
        display_stresses.clear();

        double n_factor = std::clamp(water.no3 / 5.0, 0.1, 1.0);
        double p_factor = std::clamp(water.po4 / 0.1, 0.1, 1.0);
        double k_factor = std::clamp(water.k / 2.0, 0.1, 1.0);
        double fe_factor = std::clamp(water.fe / 0.05, 0.1, 1.0);
        double co2_factor = std::clamp(water.co2 / 10.0, 0.2, 1.0);

        double limitation = std::min({n_factor, p_factor, k_factor, fe_factor, co2_factor});
        double light_limitation = limitation * (is_day ? std::max(0.1, active_light) : 0.5); // Nachts ruht die Pflanze, nimmt aber keinen Schaden!

        // Warnungen erst bei echtem, längerem Mangel (24h+)
        trackStress("Mangel: Stickstoff (NO3)", n_factor < 0.4, deltaTime_hours, 24.0);
        trackStress("Mangel: Phosphat (PO4)", p_factor < 0.4, deltaTime_hours, 24.0);
        trackStress("Mangel: Kalium (K)", k_factor < 0.4, deltaTime_hours, 24.0);
        trackStress("Mangel: Eisen (Fe)", fe_factor < 0.4, deltaTime_hours, 24.0);
        trackStress("Mangel: CO2", co2_factor < 0.4, deltaTime_hours, 24.0);

        // Lichtmangel wird nur tagsüber getrackt und erst nach 48 Stunden Dauer-Dunkelheit kritisch
        trackStress("Wenig Licht", is_day && active_light < 0.1, deltaTime_hours, 48.0);

        double env_penalty = 1.0;
        bool temp_bad = water.temp_c < (min_temp_c - 3.0) || water.temp_c > (max_temp_c + 3.0);
        bool ph_bad = water.ph < 4.5 || water.ph > 9.0;

        trackStress("Extreme Wassertemperatur", temp_bad, deltaTime_hours, 12.0);
        trackStress("Extremer pH-Wert", ph_bad, deltaTime_hours, 12.0);

        if (temp_bad) env_penalty *= 0.1;
        if (ph_bad) env_penalty *= 0.1;

        double final_growth_factor = light_limitation * env_penalty;

        // HP verlieren Pflanzen NUR noch bei extremem, anhaltendem Stress oder Totalvergiftung
        if (env_penalty < 0.5 || (is_day && active_light < 0.05 && stress_timers["Wenig Licht"] > 72.0)) {
            health_hp -= 0.2 * deltaTime_hours;
        } else {
            density_factor += 0.02 * deltaTime_hours;
            health_hp += 1.0 * deltaTime_hours;
        }

        density_factor = std::clamp(density_factor, 0.3, 1.0);
        health_hp = std::clamp(health_hp, 0.0, max_health_hp);

        double growth = base_growth_rate_cm_per_day * final_growth_factor * (deltaTime_hours / 24.0);
        current_height_cm = std::min(current_height_cm + growth, max_submersed_height_cm + (can_grow_emersed ? max_emersed_height_cm : 0.0));
        biomass_g += growth * 0.2 * density_factor;
    }
};

#endif //AQUARIUMSIM_PLANT_H