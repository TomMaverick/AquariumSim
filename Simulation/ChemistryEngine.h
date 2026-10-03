#ifndef AQUARIUMSIM_CHEMISTRYENGINE_H
#define AQUARIUMSIM_CHEMISTRYENGINE_H

#include "WaterParameters.h"
#include <cmath>
#include <algorithm> // For std::max

class ChemistryEngine {
public:
    double nitrosomonas_bacteria = 0.1; // Converts NH4 to NO2
    double nitrobacter_bacteria = 0.1; // Converts NO2 to NO3

    // Now requires bioCapacity as an argument
    void update(WaterParameters& water, double deltaTime, double bioCapacity) {
        processNitrogenCycle(water, deltaTime, bioCapacity);
        balanceAmmoniaEquilibrium(water);
        balanceCO2_pH_KH(water);
    }

private:
    void processNitrogenCycle(WaterParameters& water, double deltaTime, double bioCapacity) {
        // 1. Convert NH4 to NO2
        double convertedNH4 = water.nh4 * nitrosomonas_bacteria * deltaTime;
        if (convertedNH4 > water.nh4) {
            convertedNH4 = water.nh4;
        }
        water.nh4 -= convertedNH4;
        water.no2 += (convertedNH4 * 2.55);
        water.kh -= (convertedNH4 * 0.14);
        if (water.kh < 0.0) water.kh = 0.0;

        // Bacteria growth and death
        nitrosomonas_bacteria += (convertedNH4 * 0.05);
        nitrosomonas_bacteria -= (nitrosomonas_bacteria * 0.005 * deltaTime);

        // Bacteria growth and death
        nitrosomonas_bacteria += (convertedNH4 * 0.05);
        nitrosomonas_bacteria -= (nitrosomonas_bacteria * 0.005 * deltaTime);

        // Bacteria never die completely. A dormant biofilm always remains.
        double dormant_biofilm = 0.001;
        if (nitrosomonas_bacteria < dormant_biofilm) {
            nitrosomonas_bacteria = dormant_biofilm;
        }

        // Limit population to available surface area
        if (nitrosomonas_bacteria > bioCapacity) {
            nitrosomonas_bacteria = bioCapacity;
        }

        // 2. Convert NO2 to NO3
        double convertedNO2 = water.no2 * nitrobacter_bacteria * deltaTime;
        if (convertedNO2 > water.no2) {
            convertedNO2 = water.no2;
        }
        water.no2 -= convertedNO2;
        water.no3 += (convertedNO2 * 1.35);

        // Bacteria growth and death for Nitrobacter
        nitrobacter_bacteria += (convertedNO2 * 0.02);
        nitrobacter_bacteria -= (nitrobacter_bacteria * 0.005 * deltaTime);

        if (nitrobacter_bacteria < dormant_biofilm) {
            nitrobacter_bacteria = dormant_biofilm;
        }

        if (nitrobacter_bacteria > bioCapacity) {
            nitrobacter_bacteria = bioCapacity;
        }
    }

    void balanceAmmoniaEquilibrium(WaterParameters &water) {
        double total_ammonia_nitrogen = water.nh4 + water.nh3;
        if (total_ammonia_nitrogen <= 0.0) return; // Nothing to calculate

        // Calculate pKa based on temperature
        double pKa = 0.09018 + (2729.92 / (water.temp_c + 273.15));

        // Calculate the fraction of toxic un-ionized ammonia (NH3)
        double fraction_nh3 = 1.0 / (pow(10.0, pKa - water.ph) + 1.0);

        // Apply calculated ratio back to the water parameters
        water.nh3 = total_ammonia_nitrogen * fraction_nh3;
        water.nh4 = total_ammonia_nitrogen * (1.0 - fraction_nh3);
    }

    void balanceCO2_pH_KH(WaterParameters &water) {
        // KH must be > 0, otherwise the classic buffer equation fails (division by zero)
        if (water.kh <= 0.0) {
            // Without a KH buffer, the tank experiences an acidic crash (Old Tank Syndrome)
            water.ph = 5.0;
            return;
        }

        // CO2 from ambient air always forms a minimum baseline (approx. 0.5 mg/L).
        // Fish respiration produces additional CO2, raising this value.
        double activeCO2 = std::max(0.5, water.co2);

        // Rearranged Henderson-Hasselbalch equation to calculate current pH.
        // As dissolved CO2 increases, pH drops.
        water.ph = 7.0 - std::log10(activeCO2 / (3.0 * water.kh));

        // Humic substances / tannins slightly lower the pH further
        water.ph -= (water.humic_substances * 0.01);
    }
};

#endif //AQUARIUMSIM_CHEMISTRYENGINE_H
