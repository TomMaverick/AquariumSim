#ifndef AQUARIUMSIM_TANKMODEL_H
#define AQUARIUMSIM_TANKMODEL_H

#include <string>

struct TankModel {
    std::string name;
    double width_cm;
    double depth_cm;
    double height_cm;
    double glass_thickness_mm;

    // Fabrikmethode für den Dennerle Nano Cube 30L
    static TankModel dennerleNanoCube30() {
        return {
            "Dennerle Nano Cube 30L",
            30.0,  // Breite außen
            30.0,  // Tiefe außen
            35.0,  // Höhe außen
            5.0    // Glasstärke in mm
        };
    }

    static TankModel BreederTank_20G_High() {
        return {
            "20 Gallon Breeder Tank (High)",
            61.6,
            31.8,
            42.5,
            6.0
        };
    }

    static TankModel BreederTank_20G_Long() {
        return {
            "20 Gallon Breeder Tank (Long)",
            76.8,
            31.8,
            32.4,
            6.0
        };
    }
};

#endif //AQUARIUMSIM_TANKMODEL_H