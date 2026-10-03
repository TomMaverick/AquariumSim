#ifndef AQUARIUMSIM_HYGROPHILAPOLYSPERMA_H
#define AQUARIUMSIM_HYGROPHILAPOLYSPERMA_H

#include "Plant.h"

class HygrophilaPolyspermaRosanervig : public Plant {
public:
    HygrophilaPolyspermaRosanervig() {
        common_name = "Indischer Wasserfreund (Rosanervig)";
        scientific_name = "Hygrophila polysperma 'Rosanervig'";
        biomass_g = 6.0;
        is_floating = false;

        current_height_cm = 10.0;
        max_submersed_height_cm = 50.0;
        max_emersed_height_cm = 20.0;      // Verändert Laubform stark über Wasser
        can_grow_emersed = true;
        base_growth_rate_cm_per_day = 2.0; // Sehr schnelles Wachstum

        // Starkzehrer!
        nitrogen_consumption_mg_per_hour = 0.025;
        po4_consumption_mg_per_hour = 0.004;
        fe_consumption_mg_per_hour = 0.002;
        co2_consumption_mg_per_hour = 0.030;
        o2_production_mg_per_hour = 0.045;

        min_temp_c = 18.0; max_temp_c = 28.0;
        min_ph = 5.0; max_ph = 8.0;
    }

    void updateGrowth(const WaterParameters& water, double deltaTime_hours) override {
        Plant::updateGrowth(water, deltaTime_hours);

        // Rosa Blattadern prägen sich durch Eisen und CO2 aus
        double fe_bonus = std::clamp(water.fe / 0.08, 0.0, 1.0);
        double co2_bonus = std::clamp(water.co2 / 15.0, 0.0, 1.0);

        // Eisen hat eine etwas höhere Gewichtung für die Rosa-Färbung als CO2
        double target_color = (fe_bonus * 0.6) + (co2_bonus * 0.4);

        coloration_factor += (target_color - coloration_factor) * 0.05 * deltaTime_hours;
    }
};

#endif //AQUARIUMSIM_HYGROPHILAPOLYSPERMA_H