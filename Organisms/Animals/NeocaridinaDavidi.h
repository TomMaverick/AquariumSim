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
        o2_consumption_mg_per_hour = 0.005;
    }

    bool isSchooling() const override {
        return false; // Garnelen sind Einzelgänger / Wuseln am Boden
    }
};

#endif //AQUARIUMSIM_NEOCARIDINA_H