#ifndef AQUARIUMSIM_ANUBIASNANA_H
#define AQUARIUMSIM_ANUBIASNANA_H

#include "Plant.h"

class AnubiasNana : public Plant {
public:
    AnubiasNana() {
        common_name = "Anubias Nana";
        scientific_name = "Anubias barteri var. nana";
        biomass_g = 8.0;
        is_floating = false;

        current_height_cm = 4.0;
        max_submersed_height_cm = 15.0;     // Bleibt sehr kompakt
        max_emersed_height_cm = 10.0;       // Wächst langsam über Wasser weiter
        can_grow_emersed = true;
        base_growth_rate_cm_per_day = 0.1;  // Extrem langsames Wachstum

        nitrogen_consumption_mg_per_hour = 0.005;
        po4_consumption_mg_per_hour = 0.0005;
        fe_consumption_mg_per_hour = 0.0003;
        co2_consumption_mg_per_hour = 0.006;
        o2_production_mg_per_hour = 0.010;

        min_temp_c = 20.0; max_temp_c = 28.0;
        min_ph = 6.0; max_ph = 8.0;
    }
};

#endif //AQUARIUMSIM_ANUBIASNANA_H