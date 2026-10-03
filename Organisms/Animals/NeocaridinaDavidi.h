#ifndef AQUARIUMSIM_NEOCARIDINA_H
#define AQUARIUMSIM_NEOCARIDINA_H

#include "Animal.h"

class NeocaridinaDavidi : public Animal {
public:
    NeocaridinaDavidi() {
        common_name = "Neocaridina davidi";
        scientific_name = "Neocaridina davidi";
        biomass_g = 0.15;

        nh4_production_mg_per_hour = 0.0008;
        co2_production_mg_per_hour = 0.002;
        o2_consumption_mg_per_hour = 0.005;

        max_tolerable_nh3 = 0.01;
        max_tolerable_no2 = 0.2;
        min_tolerable_o2 = 3.0;

        // Toleranzbereich
        min_temp_c = 15.0; max_temp_c = 29.0;
        min_ph = 6.0; max_ph = 8.5;
        min_gh = 4.0; max_gh = 25.0;
        min_kh = 1.0; max_kh = 18.0;
    }

    bool isSchooling() const override { return false; }
};

#endif //AQUARIUMSIM_NEOCARIDINA_H