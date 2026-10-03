#ifndef AQUARIUMSIM_HORNWORT_H
#define AQUARIUMSIM_HORNWORT_H

#include "Plant.h"

class Hornwort : public Plant {
public:
    Hornwort() {
        common_name = "Hornwort Stem";
        scientific_name = "Ceratophyllum demersum";
        biomass_g = 3.0;

        is_floating = true;
        current_height_cm = 15.0;
        max_submersed_height_cm = 120.0;
        max_emersed_height_cm = 0.0;
        can_grow_emersed = false;
        base_growth_rate_cm_per_day = 3.0;

        nitrogen_consumption_mg_per_hour = 0.025;
        po4_consumption_mg_per_hour = 0.002;
        fe_consumption_mg_per_hour = 0.001;
        k_consumption_mg_per_hour = 0.010;
        co2_consumption_mg_per_hour = 0.020;
        o2_production_mg_per_hour = 0.035;

        min_temp_c = 10.0; max_temp_c = 32.0;
        min_ph = 5.5; max_ph = 9.0;
        min_gh = 2.0; max_gh = 30.0; // Extrem anpassungsfähig
    }
};

#endif //AQUARIUMSIM_HORNWORT_H