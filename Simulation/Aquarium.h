#ifndef AQUARIUMSIM_AQUARIUM_H
#define AQUARIUMSIM_AQUARIUM_H

#include <vector>
#include <memory>
#include <algorithm>
#include "WaterParameters.h"
#include "ChemistryEngine.h"
#include "Items.h"
#include "../Organisms/Animals/Animal.h"
#include "../Organisms/Plants/Plant.h"
#include "../Aquariums/TankModel.h"

struct SubstrateLayer { double height_cm = 5.0, porosity = 0.40; };

class Aquarium {
public:
    TankModel model;
    double volume_liters, max_capacity_liters, biological_capacity;
    WaterParameters water;
    WaterParameters water_deltas;
    ChemistryEngine chemistry;
    SubstrateLayer substrate;

    std::vector<std::unique_ptr<Animal>> livestock;
    std::vector<std::unique_ptr<Plant>> flora;

    float light_intensity = 0.8f;
    int light_spectrum = 0;
    bool co2_active = false;
    float co2_bps = 1.0f;
    bool co2_night_shutoff = true;
    bool is_day = true;

    Aquarium(const TankModel& tankModel, WaterParameters initialWater) {
        water = initialWater;
        setTankModel(tankModel);
        chemistry.nitrosomonas_bacteria = 0.001; chemistry.nitrobacter_bacteria = 0.001;
    }

    void setTankModel(const TankModel& newModel) {
        model = newModel;
        double glass_cm = model.glass_thickness_mm / 10.0;
        double i_w = model.width_cm - (2.0 * glass_cm), i_d = model.depth_cm - (2.0 * glass_cm), i_h = model.height_cm - glass_cm;
        max_capacity_liters = (i_w * i_d * (i_h - 2.0)) / 1000.0;
        double sub_l = (i_w * i_d * substrate.height_cm) / 1000.0;
        double free_l = (i_w * i_d * (i_h - 3.0 - substrate.height_cm)) / 1000.0;
        volume_liters = std::min(max_capacity_liters, free_l + (sub_l * substrate.porosity));
        biological_capacity = 0.05 + (sub_l * 0.1);
    }

    void addSpongeFilter() { biological_capacity += 0.20; }
    void addAnimal(std::unique_ptr<Animal> animal) { livestock.push_back(std::move(animal)); }
    void addPlant(std::unique_ptr<Plant> plant) { flora.push_back(std::move(plant)); }
    void addFertilizer(const Fertilizer& f, double ml) { water.no3 += f.no3_mg_per_ml * ml / volume_liters; water.po4 += f.po4_mg_per_ml * ml / volume_liters; water.k += f.k_mg_per_ml * ml / volume_liters; water.fe += f.fe_mg_per_ml * ml / volume_liters; }

    void performWaterChange(double pct, const WaterParameters& r) {
        double n = std::clamp(pct, 0.0, 100.0)/100.0, o = 1.0 - n;
        water.temp_c = water.temp_c*o + r.temp_c*n; water.nh4 = water.nh4*o + r.nh4*n;
        water.no2 = water.no2*o + r.no2*n; water.no3 = water.no3*o + r.no3*n;
        water.po4 = water.po4*o + r.po4*n; water.fe = water.fe*o + r.fe*n;
        water.k = water.k*o + r.k*n; water.o2 = water.o2*o + r.o2*n;
        water.co2 = water.co2*o + r.co2*n; water.kh = water.kh*o + r.kh*n;
        water.ca = water.ca*o + r.ca*n; water.mg = water.mg*o + r.mg*n;
    }

    void drainWater(double l) { volume_liters = std::max(2.0, volume_liters - l); }

    void topOffWater(double liters_added, const WaterParameters& sourceWater) {
        if (liters_added <= 0.0) return;
        double old_vol = volume_liters;
        double new_vol = std::min(max_capacity_liters, old_vol + liters_added);
        double actual_added = new_vol - old_vol;
        if (actual_added <= 0.0) return;

        water.temp_c = (water.temp_c * old_vol + sourceWater.temp_c * actual_added) / new_vol;
        water.kh = (water.kh * old_vol + sourceWater.kh * actual_added) / new_vol;
        water.ca = (water.ca * old_vol + sourceWater.ca * actual_added) / new_vol;
        water.mg = (water.mg * old_vol + sourceWater.mg * actual_added) / new_vol;
        water.no3 = (water.no3 * old_vol + sourceWater.no3 * actual_added) / new_vol;
        water.po4 = (water.po4 * old_vol + sourceWater.po4 * actual_added) / new_vol;
        water.fe = (water.fe * old_vol + sourceWater.fe * actual_added) / new_vol;
        water.k = (water.k * old_vol + sourceWater.k * actual_added) / new_vol;

        volume_liters = new_vol;
    }

    void update(double deltaTime, int tickCounter) {
        WaterParameters old_water = water;

        int hour = tickCounter % 24;
        is_day = (hour >= 8 && hour < 20);
        double active_light = is_day ? light_intensity : 0.0;

        // 1. Passiver Gasaustausch
        // O2 sättigt relativ schnell (0.2), CO2 gast sehr zäh aber konstant in die Raumluft aus (0.1)
        water.o2 += (8.0 - water.o2) * 0.2 * deltaTime;
        water.co2 += (0.5 - water.co2) * 0.1 * deltaTime;

        // 2. CO2 Anlage (FIXED: 1 BPS = ~216 mg/h CO2)
        if (co2_active && (is_day || !co2_night_shutoff)) {
            water.co2 += (co2_bps * 216.0 * deltaTime) / volume_liters;
        }

        // 3. Tiere & Pflanzen (Rest wie bisher)
        for (auto it = livestock.begin(); it != livestock.end();) {
            auto& a = *it;
            water.nh4 += (a->nh4_production_mg_per_hour * deltaTime) / volume_liters;
            water.co2 += (a->co2_production_mg_per_hour * deltaTime) / volume_liters;
            water.o2  -= (a->o2_consumption_mg_per_hour * deltaTime) / volume_liters;

            a->updateHealth(water, deltaTime);
            if (a->health_hp <= 0.0) it = livestock.erase(it);
            else ++it;
        }

        double n_cons = 0, p_cons = 0, fe_cons = 0, k_cons = 0;
        for (auto it = flora.begin(); it != flora.end();) {
            auto& p = *it;
            p->updateGrowth(water, active_light, is_day, deltaTime);

            if (is_day) {
                water.o2 += (p->o2_production_mg_per_hour * active_light * deltaTime) / volume_liters;
                water.co2 -= (p->co2_consumption_mg_per_hour * active_light * deltaTime) / volume_liters;
                n_cons += p->nitrogen_consumption_mg_per_hour * active_light * deltaTime;
                p_cons += p->po4_consumption_mg_per_hour * active_light * deltaTime;
                fe_cons += p->fe_consumption_mg_per_hour * active_light * deltaTime;
                k_cons += p->k_consumption_mg_per_hour * active_light * deltaTime;
            } else {
                water.o2 -= (p->o2_production_mg_per_hour * 0.2 * deltaTime) / volume_liters;
                water.co2 += (p->co2_consumption_mg_per_hour * 0.2 * deltaTime) / volume_liters;
            }

            if (p->health_hp <= 0.0) it = flora.erase(it);
            else ++it;
        }

        water.o2 = std::max(0.0, water.o2);
        water.co2 = std::max(0.0, water.co2);

        auto consume = [&](double& param, double demand_mg) {
            double demand_l = demand_mg / volume_liters;
            if (param >= demand_l) param -= demand_l;
            else param = 0.0;
        };
        double n_demand_l = n_cons / volume_liters;
        double max_nh4 = water.nh4 * 0.90;
        if (max_nh4 >= n_demand_l) water.nh4 -= n_demand_l;
        else { water.nh4 -= max_nh4; water.no3 = std::max(0.0, water.no3 - (n_demand_l - max_nh4)); }

        consume(water.po4, p_cons); consume(water.fe, fe_cons); consume(water.k, k_cons);
        chemistry.update(water, deltaTime, biological_capacity);

        // Deltas berechnen
        water_deltas.ph = water.ph - old_water.ph; water_deltas.kh = water.kh - old_water.kh;
        water_deltas.co2 = water.co2 - old_water.co2; water_deltas.o2 = water.o2 - old_water.o2;
        water_deltas.nh4 = water.nh4 - old_water.nh4; water_deltas.no2 = water.no2 - old_water.no2;
        water_deltas.no3 = water.no3 - old_water.no3; water_deltas.po4 = water.po4 - old_water.po4;
        water_deltas.k = water.k - old_water.k; water_deltas.fe = water.fe - old_water.fe;
    }
};

#endif //AQUARIUMSIM_AQUARIUM_H