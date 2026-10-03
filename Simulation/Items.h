#ifndef AQUARIUMSIM_ITEMS_H
#define AQUARIUMSIM_ITEMS_H

#include <string>

// Represents a liquid fertilizer adding nutrients (mg per ml)
struct Fertilizer {
    std::string name;
    double no3_mg_per_ml; // Nitrat
    double po4_mg_per_ml; // Phosphat
    double k_mg_per_ml;   // Kalium
    double fe_mg_per_ml;  // Eisen
};

class ItemFactory {
public:
    static Fertilizer createNPKFertilizer() {
        // Standard NPK fertilizer profile
        return {"NPK Basic", 5.0, 0.5, 3.0, 0.0};
    }

    static Fertilizer createIronFertilizer() {
        // Pure iron / micro nutrient fertilizer
        return {"Iron & Micro", 0.0, 0.0, 0.5, 1.0};
    }

    static Fertilizer createPhosphateFertilizer() {
        // Single component P-Fertilizer
        return {"Pure Phosphate", 0.0, 2.5, 0.0, 0.0};
    }
};

#endif //AQUARIUMSIM_ITEMS_H