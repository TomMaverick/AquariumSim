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
        can_grow_emersed = true;
        base_growth_rate_cm_per_day = 1.5;

        nitrogen_consumption_mg_per_hour = 0.018;
        po4_consumption_mg_per_hour = 0.003;
        fe_consumption_mg_per_hour = 0.003;
        co2_consumption_mg_per_hour = 0.035;
        o2_production_mg_per_hour = 0.040;

        min_temp_c = 18.0; max_temp_c = 30.0;
        min_ph = 5.0; max_ph = 8.0;
        min_gh = 2.0; max_gh = 20.0;
    }

    void updateGrowth(const WaterParameters& water, double active_light, bool is_day, double deltaTime_hours) override {
        Plant::updateGrowth(water, active_light, is_day, deltaTime_hours);
        double fe_bonus = std::clamp(water.fe / 0.1, 0.0, 1.0);
        double co2_bonus = std::clamp(water.co2 / 20.0, 0.0, 1.0);
        double target_color = (fe_bonus + co2_bonus) / 2.0;
        coloration_factor += (target_color - coloration_factor) * 0.05 * deltaTime_hours;
    }
};

#endif //AQUARIUMSIM_ROTALAORANGEJUICE_H