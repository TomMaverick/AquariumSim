#ifndef AQUARIUMSIM_ROTALAORANGEJUICE_H
#define AQUARIUMSIM_ROTALAORANGEJUICE_H

#include "Plant.h"

class RotalaOrangeJuice : public Plant {
public:
    RotalaOrangeJuice() {
        common_name = "Rotala Orange Juice";
        scientific_name = "Rotala rotundifolia 'Orange Juice'";
        biomass_g = 4.0;
        is_floating = false;

        current_height_cm = 8.0;
        max_submersed_height_cm = 35.0;
        max_emersed_height_cm = 15.0;
        can_grow_emersed = true; // Wächst sehr gerne aus dem Wasser
        base_growth_rate_cm_per_day = 1.5;

        nitrogen_consumption_mg_per_hour = 0.018;
        po4_consumption_mg_per_hour = 0.003;
        fe_consumption_mg_per_hour = 0.003;
        co2_consumption_mg_per_hour = 0.035;
        o2_production_mg_per_hour = 0.040;
    }

    void updateGrowth(const WaterParameters& water, double deltaTime_hours) override {
        Plant::updateGrowth(water, deltaTime_hours);

        // Die leuchtend orange Farbe kommt nur bei viel CO2 und Eisen!
        double fe_bonus = std::clamp(water.fe / 0.1, 0.0, 1.0);
        double co2_bonus = std::clamp(water.co2 / 20.0, 0.0, 1.0);
        double target_color = (fe_bonus + co2_bonus) / 2.0;

        // Langsamer Farbwechsel von Oben nach Unten (Simuliert durch Fade)
        coloration_factor += (target_color - coloration_factor) * 0.05 * deltaTime_hours;
    }
};

#endif //AQUARIUMSIM_ROTALAORANGEJUICE_H