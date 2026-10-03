#ifndef AQUARIUMSIM_WATERPARAMETERS_H
#define AQUARIUMSIM_WATERPARAMETERS_H

#include <random>
#include <algorithm>

class WaterParameters {
public:
    double temp_c = 24.0;
    double ph = 7.0;
    double ca = 20.0;
    double mg = 5.0;
    double kh = 0.0;
    double o2 = 8.0;
    double co2 = 3.0;
    double cl2 = 0.0;
    double cu = 0.0;
    double sio2 = 0.0;
    double nh4 = 0.0;
    double nh3 = 0.0;
    double no2 = 0.0;
    double no3 = 0.0;
    double fe = 0.0;
    double po4 = 0.0;
    double k = 0.0;
    double humic_substances = 0.0;

    double getGH() const {
        return (ca / 7.14) + (mg / 4.3);
    }

    double getTempF() const {
        return (temp_c * 9.0 / 5.0) + 32.0;
    }

    double getConductivity() const {
        double currentGH = getGH();
        double baseCond = (currentGH * 32.0) + (std::max(0.0, kh - currentGH) * 30.0);
        double nutrientCond = (no3 * 1.5) + (po4 * 2.0) + (k * 1.5) + (nh4 * 2.0);
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

    static bool rollChance(double probability) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(gen) <= probability;
    }

public:
    static WaterParameters createOsmosisWater() {
        WaterParameters wp;
        wp.temp_c = 15.0;
        wp.ph = 6.0;
        wp.ca = 0.0;
        wp.mg = 0.0;
        wp.kh = 0.0;
        return wp;
    }

    static WaterParameters createAverageTapWater(double desiredTemp) {
        WaterParameters wp;
        wp.temp_c = desiredTemp;
        wp.ph = getRandomVariation(7.5, 0.02);
        wp.ca = getRandomVariation(70.0, 0.10);
        wp.mg = getRandomVariation(18.0, 0.10);
        wp.kh = getRandomVariation(10.0, 0.10);
        wp.sio2 = getRandomVariation(5.0, 0.20);
        wp.no3 = getRandomVariation(15.0, 0.20);

        if (rollChance(0.10)) { wp.cu = getRandomVariation(0.08, 0.20); }
        if (rollChance(0.15)) { wp.cl2 = getRandomVariation(0.3, 0.20); }
        return wp;
    }
};

#endif //AQUARIUMSIM_WATERPARAMETERS_H