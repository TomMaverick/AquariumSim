#ifndef AQUARIUMSIM_ORGANISMS_H
#define AQUARIUMSIM_ORGANISMS_H

#include <string>

struct Animal {
    std::string species_name;
    double nh4_production_mg_per_hour;
    bool forms_schools = false;        // NEU: Bildet Gruppen / Schwärme
};

struct Plant {
    std::string species_name;
    double nitrogen_consumption_mg_per_hour;
};

class OrganismFactory {
public:
    static Animal createNeocaridina() {
        return {"Neocaridina Davidi", 0.0008, false};
    }

    static Animal createNeonTetra() {
        return {"Paracheirodon innesi (Neon Tetra)", 0.015, true}; // Schwarmfisch!
    }

    static Animal createGoldfish() {
        return {"Carassius auratus (Goldfish)", 0.25, false};
    }

    static Plant createHornwortStem() {
        return {"Ceratophyllum demersum (Single Stem)", 0.015};
    }

    static Plant createAnubias() {
        return {"Anubias barteri (Rhizome)", 0.005};
    }
};

#endif //AQUARIUMSIM_ORGANISMS_H