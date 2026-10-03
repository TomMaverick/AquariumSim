#ifndef AQUARIUMSIM_WATERPARAMETERS_H
#define AQUARIUMSIM_WATERPARAMETERS_H

#include <random>
#include <algorithm> // For std::max

class WaterParameters {
public:
    // Physical properties
    double temp_c = 24.0;

    // Basic chemical properties
    double ph = 7.0;
    double ca = 20.0;          // Calcium (mg/L)
    double mg = 5.0;           // Magnesium (mg/L)
    double kh = 0.0;           // Carbonate hardness (dKH)

    // Dissolved gases (mg/L)
    double o2 = 8.0;
    double co2 = 3.0;

    // Toxins & Metals (mg/L)
    double cl2 = 0.0;
    double cu = 0.0;
    double sio2 = 0.0;

    // Nitrogen cycle (mg/L)
    double nh4 = 0.0;
    double nh3 = 0.0;
    double no2 = 0.0;
    double no3 = 0.0;

    // Nutrients & Organics (mg/L)
    double fe = 0.0;
    double po4 = 0.0;
    double k = 0.0;
    double humic_substances = 0.0;

    // --- DYNAMIC PROPERTIES ---

    // Calculates General Hardness (dGH) based on Calcium and Magnesium
    double getGH() const {
        return (ca / 7.14) + (mg / 4.3);
    }

    double getTempF() const {
        return (temp_c * 9.0 / 5.0) + 32.0;
    }

    // Calculates electrical conductivity (µS/cm) in real-time based on dissolved solids
    double getConductivity() const {
        double currentGH = getGH();

        // 1 dGH adds approx 32 µS/cm.
        // If KH > GH (e.g., sodium bicarbonate presence), add the surplus
        double baseCond = (currentGH * 32.0) + (std::max(0.0, kh - currentGH) * 30.0);

        // Fertilizers and waste products slightly increase conductivity
        double nutrientCond = (no3 * 1.5) + (po4 * 2.0) + (k * 1.5) + (nh4 * 2.0);

        // Acids/tannins and other trace elements
        double traceCond = (humic_substances * 0.5) + (sio2 * 1.0);

        return baseCond + nutrientCond + traceCond;
    }

private:
    static double getRandomVariation(double baseValue, double variationPercent) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(baseValue * (1.0 - variationPercent),
                                                    baseValue * (1.0 + variationPercent));
        return dist(gen);
    }

    // Rolls a probability check (e.g., 0.10 for 10% chance)
    static bool rollChance(double probability) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(gen) <= probability;
    }

public:
    // --- FACTORY METHODS ---

    // Osmosis systems usually run on the cold water tap (protects the membrane)
    static WaterParameters createOsmosisWater() {
        WaterParameters wp;
        wp.temp_c = 15.0; // Needs to be heated by the player
        wp.ph = 6.0;
        wp.ca = 0.0;
        wp.mg = 0.0;
        wp.kh = 0.0;
        return wp;
    }

    // Player selects the desired temperature at the tap
    static WaterParameters createAverageTapWater(double desiredTemp) {
        WaterParameters wp;
        wp.temp_c = desiredTemp;

        wp.ph = getRandomVariation(7.5, 0.02);

        // Generating typical tap water hardness (~14 dGH total)
        wp.ca = getRandomVariation(70.0, 0.10); // Approx 9.8 dGH
        wp.mg = getRandomVariation(18.0, 0.10); // Approx 4.1 dGH

        wp.kh = getRandomVariation(10.0, 0.10);

        wp.sio2 = getRandomVariation(5.0, 0.20);
        wp.no3 = getRandomVariation(15.0, 0.20);

        // Toxins only appear in specific households/situations
        if (rollChance(0.10)) { // 10% chance for copper from old pipes
            wp.cu = getRandomVariation(0.08, 0.20);
        }
        if (rollChance(0.15)) { // 15% chance for chlorinated water
            wp.cl2 = getRandomVariation(0.3, 0.20);
        }

        return wp;
    }
};

#endif //AQUARIUMSIM_WATERPARAMETERS_H