#ifndef AQUARIUMSIM_ANIMAL_H
#define AQUARIUMSIM_ANIMAL_H

#include <string>

class Animal {
public:
    std::string common_name;
    std::string scientific_name;
    double biomass_g;
    double health_hp = 100.0;
    double age_days = 0.0;

    double nh4_production_mg_per_hour;
    double o2_consumption_mg_per_hour;

    // Virtueller Destruktor für saubere Polymorphie
    virtual ~Animal() = default;

    // Reine virtuelle Methode: Jedes Tier implementiert sein eigenes Bewegungsverhalten in der 2D-Ansicht
    virtual bool isSchooling() const = 0;
};

#endif //AQUARIUMSIM_ANIMAL_H