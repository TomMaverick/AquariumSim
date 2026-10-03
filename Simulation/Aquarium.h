#ifndef AQUARIUMSIM_AQUARIUM_H
#define AQUARIUMSIM_AQUARIUM_H

#include <vector>
#include "WaterParameters.h"
#include "ChemistryEngine.h"
#include "Organisms.h"

class Aquarium {
public:
    double volume_liters;
    double biological_capacity;

    WaterParameters water;
    ChemistryEngine chemistry;

    // Lists of living organisms in the tank
    std::vector<Animal> livestock;
    std::vector<Plant> flora;

    Aquarium(double volume, WaterParameters initialWater) {
        volume_liters = volume;
        water = initialWater;
        biological_capacity = 0.01;

        chemistry.nitrosomonas_bacteria = 0.001;
        chemistry.nitrobacter_bacteria = 0.001;
    }

    // --- PLAYER ACTIONS ---

    void addSpongeFilter() {
        biological_capacity += 0.20;
    }

    void addAnimal(const Animal& animal) {
        livestock.push_back(animal);
    }

    void addPlant(const Plant& plant) {
        flora.push_back(plant);
    }

    // --- GAME LOOP ---

    void update(double deltaTime) {
        // 1. Animals produce waste (NH4)
        double total_nh4_produced_mg = 0.0;
        for (const auto& animal : livestock) {
            total_nh4_produced_mg += animal.nh4_production_mg_per_hour * deltaTime;
        }
        // Convert absolute mass to concentration (mg/L) and add to water
        water.nh4 += (total_nh4_produced_mg / volume_liters);

        // 2. Plants consume nutrients
        double total_n_consumed_mg = 0.0;
        for (const auto& plant : flora) {
            total_n_consumed_mg += plant.nitrogen_consumption_mg_per_hour * deltaTime;
        }
        double n_consumed_mg_l = total_n_consumed_mg / volume_liters;

        // Plants cannot magically suck the water entirely dry of NH4 instantly.
        // They can only consume up to 90% of the currently freely floating NH4 per tick,
        // leaving at least a trace amount for filter bacteria to stay slightly active.
        double max_accessible_nh4 = water.nh4 * 0.90;

        if (max_accessible_nh4 >= n_consumed_mg_l) {
            water.nh4 -= n_consumed_mg_l;
        } else {
            // Plants take whatever NH4 they can reach
            double remaining_demand = n_consumed_mg_l - max_accessible_nh4;
            water.nh4 -= max_accessible_nh4;

            // And pull the rest of their demand from Nitrate
            water.no3 -= remaining_demand;
            if (water.no3 < 0.0) water.no3 = 0.0;
        }

        // 3. Filter bacteria process whatever is left in the water column
        chemistry.update(water, deltaTime, biological_capacity);
    }
};

#endif //AQUARIUMSIM_AQUARIUM_H