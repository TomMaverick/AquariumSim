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

struct SubstrateLayer {
    double height_cm = 5.0;
    double porosity = 0.40;
};

class Aquarium {
public:
    TankModel model;
    double initial_gross_volume;
    double volume_liters;
    double max_capacity_liters;
    double biological_capacity;

    WaterParameters water;
    ChemistryEngine chemistry;
    SubstrateLayer substrate;

    std::vector<std::unique_ptr<Animal>> livestock;
    std::vector<std::unique_ptr<Plant>> flora;

    Aquarium(const TankModel& tankModel, WaterParameters initialWater) {
        water = initialWater;
        setTankModel(tankModel);
        chemistry.nitrosomonas_bacteria = 0.001;
        chemistry.nitrobacter_bacteria = 0.001;
    }

    void setTankModel(const TankModel& newModel) {
        model = newModel;

        double glass_cm = model.glass_thickness_mm / 10.0;
        double inner_width = model.width_cm - (2.0 * glass_cm);
        double inner_depth = model.depth_cm - (2.0 * glass_cm);
        double inner_height = model.height_cm - glass_cm;

        double max_water_h = inner_height - 2.0;
        max_capacity_liters = (inner_width * inner_depth * max_water_h) / 1000.0;

        double current_water_height = inner_height - 3.0;
        double gross_water_l = (inner_width * inner_depth * current_water_height) / 1000.0;

        double substrate_bulk_l = (inner_width * inner_depth * substrate.height_cm) / 1000.0;
        double free_water_h = current_water_height - substrate.height_cm;
        double free_water_l = (inner_width * inner_depth * free_water_h) / 1000.0;

        volume_liters = std::min(max_capacity_liters, free_water_l + (substrate_bulk_l * substrate.porosity));
        initial_gross_volume = gross_water_l;
        biological_capacity = 0.05 + (substrate_bulk_l * 0.1);
    }

    void addSpongeFilter() { biological_capacity += 0.20; }
    void addAnimal(std::unique_ptr<Animal> animal) { livestock.push_back(std::move(animal)); }
    void addPlant(std::unique_ptr<Plant> plant) { flora.push_back(std::move(plant)); }

    void addFertilizer(const Fertilizer& fert, double amount_ml) {
        double concentration_factor = amount_ml / volume_liters;
        water.no3 += fert.no3_mg_per_ml * concentration_factor;
        water.po4 += fert.po4_mg_per_ml * concentration_factor;
        water.k   += fert.k_mg_per_ml * concentration_factor;
        water.fe  += fert.fe_mg_per_ml * concentration_factor;
    }

    void performWaterChange(double percentage, const WaterParameters& replacementWater) {
        double ratioNew = std::clamp(percentage, 0.0, 100.0) / 100.0;
        double ratioOld = 1.0 - ratioNew;

        water.temp_c = (water.temp_c * ratioOld) + (replacementWater.temp_c * ratioNew);
        water.ca = (water.ca * ratioOld) + (replacementWater.ca * ratioNew);
        water.mg = (water.mg * ratioOld) + (replacementWater.mg * ratioNew);
        water.kh = (water.kh * ratioOld) + (replacementWater.kh * ratioNew);
        water.nh4 = (water.nh4 * ratioOld) + (replacementWater.nh4 * ratioNew);
        water.nh3 = (water.nh3 * ratioOld) + (replacementWater.nh3 * ratioNew);
        water.no2 = (water.no2 * ratioOld) + (replacementWater.no2 * ratioNew);
        water.no3 = (water.no3 * ratioOld) + (replacementWater.no3 * ratioNew);
        water.po4 = (water.po4 * ratioOld) + (replacementWater.po4 * ratioNew);
        water.fe = (water.fe * ratioOld) + (replacementWater.fe * ratioNew);
        water.k = (water.k * ratioOld) + (replacementWater.k * ratioNew);
        water.cl2 = (water.cl2 * ratioOld) + (replacementWater.cl2 * ratioNew);
        water.cu = (water.cu * ratioOld) + (replacementWater.cu * ratioNew);
        water.sio2 = (water.sio2 * ratioOld) + (replacementWater.sio2 * ratioNew);
        water.humic_substances = (water.humic_substances * ratioOld) + (replacementWater.humic_substances * ratioNew);

        volume_liters = std::min(max_capacity_liters, volume_liters);
    }

    void topOffWater(double liters_added, const WaterParameters& sourceWater) {
        if (liters_added <= 0.0) return;
        double old_vol = volume_liters;
        double new_vol = std::min(max_capacity_liters, old_vol + liters_added);
        double actual_added = new_vol - old_vol;
        if (actual_added <= 0.0) return;

        water.temp_c = (water.temp_c * old_vol + sourceWater.temp_c * actual_added) / new_vol;
        water.ca = (water.ca * old_vol + sourceWater.ca * actual_added) / new_vol;
        water.mg = (water.mg * old_vol + sourceWater.mg * actual_added) / new_vol;
        water.kh = (water.kh * old_vol + sourceWater.kh * actual_added) / new_vol;
        water.nh4 = (water.nh4 * old_vol + sourceWater.nh4 * actual_added) / new_vol;
        water.nh3 = (water.nh3 * old_vol + sourceWater.nh3 * actual_added) / new_vol;
        water.no2 = (water.no2 * old_vol + sourceWater.no2 * actual_added) / new_vol;
        water.no3 = (water.no3 * old_vol + sourceWater.no3 * actual_added) / new_vol;
        water.po4 = (water.po4 * old_vol + sourceWater.po4 * actual_added) / new_vol;
        water.fe = (water.fe * old_vol + sourceWater.fe * actual_added) / new_vol;
        water.k = (water.k * old_vol + sourceWater.k * actual_added) / new_vol;
        water.cl2 = (water.cl2 * old_vol + sourceWater.cl2 * actual_added) / new_vol; // KORRIGIERT: ratioNew zu actual_added
        water.cu = (water.cu * old_vol + sourceWater.cu * actual_added) / new_vol;
        water.sio2 = (water.sio2 * old_vol + sourceWater.sio2 * actual_added) / new_vol;
        water.humic_substances = (water.humic_substances * old_vol + sourceWater.humic_substances * actual_added) / new_vol;

        volume_liters = new_vol;
    }

    void drainWater(double liters_removed) {
        if (liters_removed <= 0.0) return;
        volume_liters -= liters_removed;
        if (volume_liters < 2.0) volume_liters = 2.0;
    }

    void update(double deltaTime) {
        double total_nh4_produced_mg = 0.0;
        for (const auto& animal : livestock) {
            total_nh4_produced_mg += animal->nh4_production_mg_per_hour * deltaTime;
        }
        water.nh4 += (total_nh4_produced_mg / volume_liters);

        double total_n_consumed_mg = 0.0;
        for (const auto& plant : flora) {
            plant->updateGrowth(water, deltaTime);
            total_n_consumed_mg += plant->nitrogen_consumption_mg_per_hour * deltaTime;
        }

        double n_consumed_mg_l = total_n_consumed_mg / volume_liters;

        double max_accessible_nh4 = water.nh4 * 0.90;
        if (max_accessible_nh4 >= n_consumed_mg_l) {
            water.nh4 -= n_consumed_mg_l;
        } else {
            double remaining_demand = n_consumed_mg_l - max_accessible_nh4;
            water.nh4 -= max_accessible_nh4;
            water.no3 -= remaining_demand;
            if (water.no3 < 0.0) water.no3 = 0.0;
        }

        chemistry.update(water, deltaTime, biological_capacity);
    }
};

#endif //AQUARIUMSIM_AQUARIUM_H