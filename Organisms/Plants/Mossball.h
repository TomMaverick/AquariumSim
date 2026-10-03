#ifndef AQUARIUMSIM_MOSSBALL_H
#define AQUARIUMSIM_MOSSBALL_H

#include "Plant.h"

class MossBall : public Plant {
public:
    MossBall() {
        common_name = "Marimo Moosball";
        scientific_name = "Aegagropila linnaei";
        biomass_g = 15.0;
        is_floating = false;

        current_height_cm = 4.0;
        max_submersed_height_cm = 12.0;
        max_emersed_height_cm = 0.0;
        can_grow_emersed = false;
        base_growth_rate_cm_per_day = 0.05;

        nitrogen_consumption_mg_per_hour = 0.002;
        po4_consumption_mg_per_hour = 0.0002;
        fe_consumption_mg_per_hour = 0.0001;
        co2_consumption_mg_per_hour = 0.003;
        o2_production_mg_per_hour = 0.005;

        min_temp_c = 4.0; max_temp_c = 28.0; // Verträgt auch kühles Wasser exzellent
        min_ph = 6.0; max_ph = 8.5;
        min_gh = 2.0; max_gh = 25.0;
    }

    void updateGrowth(const WaterParameters& water, double active_light, bool is_day, double deltaTime_hours) override {
        Plant::updateGrowth(water, active_light, is_day, deltaTime_hours);
        density_factor = 1.0;
    }
};

#endif //AQUARIUMSIM_MOSSBALL_H