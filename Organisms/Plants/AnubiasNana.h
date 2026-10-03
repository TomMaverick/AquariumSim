#ifndef AQUARIUMSIM_ANUBIASNANA_H
#define AQUARIUMSIM_ANUBIASNANA_H

#include "Plant.h"

class AnubiasNana : public Plant {
public:
    AnubiasNana() {
        common_name = "Anubias Nana";
        scientific_name = "Anubias barteri var. nana";
        biomass_g = 8.0;
        is_floating = false;

        current_height_cm = 4.0;
        max_submersed_height_cm = 15.0;
        max_emersed_height_cm = 10.0;
        can_grow_emersed = true;
        base_growth_rate_cm_per_day = 0.1;

        nitrogen_consumption_mg_per_hour = 0.005;
        po4_consumption_mg_per_hour = 0.0005;
        fe_consumption_mg_per_hour = 0.0003;
        k_consumption_mg_per_hour = 0.001;
        co2_consumption_mg_per_hour = 0.006;
        o2_production_mg_per_hour = 0.010;

        min_temp_c = 15.0; max_temp_c = 30.0;
        min_ph = 5.5; max_ph = 8.5;
        min_gh = 1.0; max_gh = 25.0; // Äußerst robust
    }
};

#endif //AQUARIUMSIM_ANUBIASNANA_H