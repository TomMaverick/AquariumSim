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

        current_height_cm = 4.0; // Entspricht hier dem Kugel-Durchmesser
        max_submersed_height_cm = 12.0;
        max_emersed_height_cm = 0.0;
        can_grow_emersed = false; // Stirbt sofort an der Luft
        base_growth_rate_cm_per_day = 0.05; // Wächst unmerklich langsam

        nitrogen_consumption_mg_per_hour = 0.002;
        po4_consumption_mg_per_hour = 0.0002;
        fe_consumption_mg_per_hour = 0.0001;
        co2_consumption_mg_per_hour = 0.003;
        o2_production_mg_per_hour = 0.005;

        min_temp_c = 5.0; max_temp_c = 25.0;
        min_ph = 6.0; max_ph = 8.0;
    }

    void updateGrowth(const WaterParameters& water, double deltaTime_hours) override {
        Plant::updateGrowth(water, deltaTime_hours);

        // Ein Moosball geilt nicht aus, er bleibt immer zu 100% kompakt
        density_factor = 1.0;
    }
};

#endif //AQUARIUMSIM_MOSSBALL_H