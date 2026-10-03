#ifndef AQUARIUMSIM_NEONTETRA_H
#define AQUARIUMSIM_NEONTETRA_H

#include "Animal.h"

class NeonTetra : public Animal {
public:
    NeonTetra() {
        common_name = "Neon Tetra";
        scientific_name = "Paracheirodon innesi";
        biomass_g = 0.5;

        nh4_production_mg_per_hour = 0.015;
        co2_production_mg_per_hour = 0.025;
        o2_consumption_mg_per_hour = 0.040;

        max_tolerable_nh3 = 0.02;
        max_tolerable_no2 = 0.5;
        min_tolerable_o2 = 4.0;

        // Toleranzbereich (Deckt normales Leitungswasser problemlos ab)
        min_temp_c = 18.0; max_temp_c = 28.0;
        min_ph = 4.8; max_ph = 7.8;
        min_gh = 1.0; max_gh = 18.0;
        min_kh = 0.0; max_kh = 12.0;
    }

    bool isSchooling() const override { return true; }
};

#endif //AQUARIUMSIM_NEONTETRA_H