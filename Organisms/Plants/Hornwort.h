#ifndef AQUARIUMSIM_HORNWORT_H
#define AQUARIUMSIM_HORNWORT_H

#include "Plant.h"

class Hornwort : public Plant {
public:
    Hornwort() {
        common_name = "Hornwort Stem";
        scientific_name = "Ceratophyllum demersum";
        biomass_g = 3.0;

        is_floating = true; // Schwimmt an der Oberfläche!
        current_height_cm = 15.0; // Höhe ist hier die "Länge" der Ranke
        max_submersed_height_cm = 120.0;
        max_emersed_height_cm = 0.0;
        can_grow_emersed = false; // Kann physikalisch nicht an der Luft stehen, trocknet aber als Schwimmpflanze dank Kapillareffekt nicht aus
        base_growth_rate_cm_per_day = 3.0; // Extrem schnelles Wachstum

        nitrogen_consumption_mg_per_hour = 0.025;
        po4_consumption_mg_per_hour = 0.002;
        fe_consumption_mg_per_hour = 0.001;
        co2_consumption_mg_per_hour = 0.020;
    }
};

#endif //AQUARIUMSIM_HORNWORT_H