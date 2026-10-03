#ifndef AQUARIUMSIM_NEONTETRA_H
#define AQUARIUMSIM_NEONTETRA_H

#include "Animal.h"

class NeonTetra : public Animal {
public:
    NeonTetra() {
        common_name = "Neon Tetra";
        scientific_name = "Paracheirodon innesi";
        biomass_g = 0.5;
        nh4_production_mg_per_hour = 0.015;
        o2_consumption_mg_per_hour = 0.040;
    }

    bool isSchooling() const override {
        return true; // Neon Tetras bilden Schwärme
    }
};

#endif //AQUARIUMSIM_NEONTETRA_H