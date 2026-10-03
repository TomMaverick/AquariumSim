#ifndef AQUARIUMSIM_CRYPTOCORYNEWENDTII_H
#define AQUARIUMSIM_CRYPTOCORYNEWENDTII_H

#include "Plant.h"

class CryptocoryneWendtii : public Plant {
public:
    CryptocoryneWendtii() {
        common_name = "Wendt's Wasserkelch (Green)";
        scientific_name = "Cryptocoryne wendtii 'green'";
        biomass_g = 10.0;
        is_floating = false;

        current_height_cm = 6.0;
        max_submersed_height_cm = 20.0;
        max_emersed_height_cm = 10.0;      // Klassische Sumpfpflanze
        can_grow_emersed = true;
        base_growth_rate_cm_per_day = 0.3; // Moderates Rosetten-Wachstum

        nitrogen_consumption_mg_per_hour = 0.008;
        po4_consumption_mg_per_hour = 0.001;
        fe_consumption_mg_per_hour = 0.0005;
        co2_consumption_mg_per_hour = 0.010;
        o2_production_mg_per_hour = 0.015;

        min_temp_c = 20.0; max_temp_c = 28.0;
        min_ph = 5.5; max_ph = 8.0;
    }
};

#endif //AQUARIUMSIM_CRYPTOCORYNEWENDTII_H