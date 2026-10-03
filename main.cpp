#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include "Simulation/WaterParameters.h"
#include "Simulation/ChemistryEngine.h"
#include "Simulation/Aquarium.h"

// Hilfsfunktion für die Konsolenausgabe
void printStatus(int hour, const Aquarium& tank) {
    std::cout << std::fixed << std::setprecision(4) << std::left
              << std::setw(8)  << hour
              << std::setw(8)  << tank.water.ph
              << std::setw(8)  << tank.water.kh
              << std::setw(12) << tank.water.nh4
              << std::setw(12) << tank.water.nh3
              << std::setw(12) << tank.water.no2
              << std::setw(12) << tank.water.no3
              << std::setw(15) << tank.chemistry.nitrosomonas_bacteria
              << std::setw(15) << tank.chemistry.nitrobacter_bacteria << "\n";
}

int main() {
    // 1. Setup the Aquarium
    WaterParameters initialWater = WaterParameters::createAverageTapWater(22.0);

    // Für diesen Test entfernen wir Nitrat und Ammonium komplett aus dem Leitungswasser
    initialWater.nh4 = 1.0;
    initialWater.no3 = 10.0;

    Aquarium nanoCube(30.0, initialWater);
    nanoCube.addSpongeFilter();

    // 2. Pflanzen hinzufügen (Gesamtverbrauch = 0.020 mg/h)
    nanoCube.addPlant(OrganismFactory::createHornwortStem()); // 0.015 mg/h
    nanoCube.addPlant(OrganismFactory::createAnubias());      // 0.005 mg/h

    // 3. Garnelen hinzufügen (25 Stück * 0.0008 mg/h = 0.020 mg/h Produktion)
    for(int i = 0; i < 26; i++) {
        nanoCube.addAnimal(OrganismFactory::createNeocaridina());
    }

    int tickCounter = 0;
    bool isRunning = true;
    double deltaTime = 1.0;

    std::cout << "--- Dennerle Nano Cube 30L Simulation Started ---\n";
    std::cout << "Setup: Tap water, Sponge Filter, 25x Neocaridina, 1x Hornwort Stem, 1x Anubias\n";
    std::cout << "Target: Perfect balance (Shrimp NH4 production == Plant consumption)\n\n";

    std::cout << std::left
              << std::setw(8)  << "Hour"
              << std::setw(8)  << "pH"
              << std::setw(8)  << "KH"
              << std::setw(12) << "NH4 (mg/L)"
              << std::setw(12) << "NH3 (mg/L)"
              << std::setw(12) << "NO2 (mg/L)"
              << std::setw(12) << "NO3 (mg/L)"
              << std::setw(15) << "Nitrosomonas"
              << std::setw(15) << "Nitrobacter" << "\n";
    std::cout << "------------------------------------------------------------------------------------------------------\n";

    // Stunde 0 ausgeben
    nanoCube.chemistry.update(nanoCube.water, 0.0, nanoCube.biological_capacity);
    printStatus(tickCounter, nanoCube);

    while (isRunning) {
        tickCounter++;
        nanoCube.update(deltaTime);
        printStatus(tickCounter, nanoCube);

        if (tickCounter >= 2400) {
            isRunning = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    return 0;
}