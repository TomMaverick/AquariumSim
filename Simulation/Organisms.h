#ifndef AQUARIUMSIM_ORGANISMS_H
#define AQUARIUMSIM_ORGANISMS_H

#include <string>

// Represents animals that consume oxygen and produce waste
struct Animal {
    std::string species_name;
    double nh4_production_mg_per_hour; // Absolute mass of NH4 produced per hour
};

// Represents plants that absorb nutrients and produce oxygen
struct Plant {
    std::string species_name;
    double nitrogen_consumption_mg_per_hour; // Absolute mass of Nitrogen consumed per hour
};

class OrganismFactory {
public:
    // --- ANIMALS ---

    // Micro-Bioload: Dwarf shrimp
    static Animal createNeocaridina() {
        return {"Neocaridina Davidi", 0.0008};
    }

    // Low-Bioload: Small schooling fish
    static Animal createNeonTetra() {
        return {"Paracheirodon innesi (Neon Tetra)", 0.015};
    }

    // High-Bioload: Large waste producers
    static Animal createGoldfish() {
        return {"Carassius auratus (Goldfish)", 0.25};
    }

    // --- PLANTS ---

    // Fast-Grower: A single stem of Hornwort.
    // Players can plant multiple individual stems to increase consumption.
    static Plant createHornwortStem() {
        return {"Ceratophyllum demersum (Single Stem)", 0.015};
    }

    // Slow-Grower: Anubias plant (Rhizome). Very low nutrient consumption.
    static Plant createAnubias() {
        return {"Anubias barteri (Rhizome)", 0.005};
    }
};

#endif //AQUARIUMSIM_ORGANISMS_H