#ifndef AQUARIUMSIM_ANIMAL_H
#define AQUARIUMSIM_ANIMAL_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "../../Simulation/WaterParameters.h"

class Animal {
public:
    std::string common_name, scientific_name;
    double biomass_g, max_health_hp = 100.0, health_hp = 100.0, age_days = 0.0;
    bool is_suffocating = false;

    // Diagnose-System mit Timern
    std::map<std::string, double> stress_timers;
    std::vector<std::string> display_stresses;

    double nh4_production_mg_per_hour, o2_consumption_mg_per_hour, co2_production_mg_per_hour;
    double max_tolerable_nh3 = 0.02, max_tolerable_no2 = 0.2, min_tolerable_o2 = 4.0;
    double min_temp_c = 20.0, max_temp_c = 26.0, min_ph = 6.5, max_ph = 7.5;
    double min_gh = 5.0, max_gh = 15.0, min_kh = 3.0, max_kh = 10.0;

    virtual ~Animal() = default;
    virtual bool isSchooling() const = 0;

    void trackStress(const std::string& name, bool condition, double deltaTime, double threshold_hours) {
        if (condition) {
            stress_timers[name] += deltaTime;
        } else {
            // Erholt sich doppelt so schnell, wie es gestresst wurde
            stress_timers[name] = std::max(0.0, stress_timers[name] - deltaTime * 2.0);
        }
        if (stress_timers[name] > threshold_hours) display_stresses.push_back(name);
    }

    virtual void updateHealth(const WaterParameters& water, double deltaTime_hours) {
        age_days += deltaTime_hours / 24.0;
        double damage = 0.0;
        display_stresses.clear();

        // Akute Vergiftungen (Schwellenwert 0.0h -> Sofortige Warnung)
        bool nh3_tox = water.nh3 > max_tolerable_nh3;
        bool no2_tox = water.no2 > max_tolerable_no2;
        is_suffocating = water.o2 < min_tolerable_o2;

        trackStress("Ammoniak-Vergiftung (Kiemen verätzt)!", nh3_tox, deltaTime_hours, 0.0);
        trackStress("Nitrit-Vergiftung (Innere Erstickung)!", no2_tox, deltaTime_hours, 0.0);
        trackStress("Sauerstoffmangel (Erstickt)!", is_suffocating, deltaTime_hours, 0.0);

        if (nh3_tox) damage += (water.nh3 - max_tolerable_nh3) * 50.0;
        if (no2_tox) damage += (water.no2 - max_tolerable_no2) * 20.0;
        if (is_suffocating) damage += (min_tolerable_o2 - water.o2) * 10.0;

        // Schleichender Stress (Warnung erst nach X Stunden Dauerbelastung)
        bool temp_low = water.temp_c < min_temp_c, temp_high = water.temp_c > max_temp_c;
        bool ph_low = water.ph < min_ph, ph_high = water.ph > max_ph;
        bool gh_low = water.getGH() < min_gh, gh_high = water.getGH() > max_gh;

        // Toleranzen ausreizen (Temperatur warnt nach 2h, Wasserwerte nach 12h)
        trackStress("Zu kalt", temp_low, deltaTime_hours, 2.0);
        trackStress("Zu heiß", temp_high, deltaTime_hours, 2.0);
        trackStress("Säuresturz / pH zu tief", ph_low, deltaTime_hours, 12.0);
        trackStress("pH zu hoch (Alkalisch)", ph_high, deltaTime_hours, 12.0);
        trackStress("Wasser zu weich (GH)", gh_low, deltaTime_hours, 24.0);
        trackStress("Wasser zu hart (GH)", gh_high, deltaTime_hours, 24.0);

        if (temp_low) damage += (min_temp_c - water.temp_c) * 1.5;
        if (temp_high) damage += (water.temp_c - max_temp_c) * 1.5;
        if (ph_low) damage += (min_ph - water.ph) * 5.0;
        if (ph_high) damage += (water.ph - max_ph) * 5.0;

        if (damage > 0.0) {
            health_hp -= damage * deltaTime_hours;
            max_health_hp -= damage * 0.1 * deltaTime_hours; // Permanenter Schaden
        } else {
            health_hp += 2.0 * deltaTime_hours;
        }

        health_hp = std::clamp(health_hp, 0.0, max_health_hp);
        max_health_hp = std::max(max_health_hp, 0.0);
    }
};

#endif //AQUARIUMSIM_ANIMAL_H