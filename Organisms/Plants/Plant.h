#ifndef AQUARIUMSIM_PLANT_H
#define AQUARIUMSIM_PLANT_H

#include <string>
#include <algorithm>
#include "../../Simulation/WaterParameters.h"

class Plant {
public:
    std::string common_name;
    std::string scientific_name;

    double biomass_g;
    double health_hp = 100.0;
    double age_days = 0.0;
    bool is_floating = false;

    // Wachstums-Parameter
    double current_height_cm = 5.0;
    double max_submersed_height_cm = 30.0;
    double max_emersed_height_cm = 10.0;
    bool can_grow_emersed = true;
    double base_growth_rate_cm_per_day = 1.0;

    // Visuelle Faktoren [0.0 - 1.0]
    double coloration_factor = 0.0; // 0.0 = blassgrün, 1.0 = intensiv rot/orange/pink
    double density_factor = 1.0;    // 1.0 = kompakt, 0.3 = extrem gegeilt (große Blattabstände)

    // Stoffwechsel (mg pro Stunde)
    double nitrogen_consumption_mg_per_hour = 0.0;
    double po4_consumption_mg_per_hour = 0.0;
    double fe_consumption_mg_per_hour = 0.0;
    double co2_consumption_mg_per_hour = 0.0;
    double o2_production_mg_per_hour = 0.0;

    double min_temp_c = 20.0, max_temp_c = 28.0;
    double min_ph = 6.0, max_ph = 8.0;

    virtual ~Plant() = default;

    // Berechnet das tägliche Wachstum anhand des Liebigschen Minimumgesetzes
    virtual void updateGrowth(const WaterParameters& water, double deltaTime_hours) {
        age_days += deltaTime_hours / 24.0;

        // 1. Minimumgesetz (Was fehlt der Pflanze am meisten?)
        double n_factor = std::clamp(water.no3 / 5.0, 0.1, 1.0);
        double p_factor = std::clamp(water.po4 / 0.1, 0.1, 1.0);
        double fe_factor = std::clamp(water.fe / 0.05, 0.1, 1.0);
        double co2_factor = std::clamp(water.co2 / 10.0, 0.2, 1.0);

        double limitation = std::min({n_factor, p_factor, fe_factor, co2_factor});

        // 2. Geilwuchs simulieren (Reckt sich nach Licht/Nährstoffen, Blätter stehen weit auseinander)
        if (limitation < 0.5) {
            density_factor -= 0.02 * deltaTime_hours; // Wird "spargelig"
        } else {
            density_factor += 0.05 * deltaTime_hours; // Wächst kompakt
        }
        density_factor = std::clamp(density_factor, 0.3, 1.0);

        // 3. Echtes Wachstum in cm berechnen
        double growth = base_growth_rate_cm_per_day * limitation * (deltaTime_hours / 24.0);
        current_height_cm += growth;

        // Maximalhöhe begrenzen
        double absolute_max = max_submersed_height_cm + (can_grow_emersed ? max_emersed_height_cm : 0.0);
        if (current_height_cm > absolute_max) {
            current_height_cm = absolute_max;
        }

        // Biomasse skaliert mit der Höhe UND der Dichte
        biomass_g += growth * 0.2 * density_factor;
    }
};

#endif //AQUARIUMSIM_PLANT_H