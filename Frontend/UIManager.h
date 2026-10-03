#ifndef AQUARIUMSIM_UIMANAGER_H
#define AQUARIUMSIM_UIMANAGER_H

#include "../Simulation/Aquarium.h"
#include "../Simulation/Items.h"
#include "../Organisms/OrganismFactory.h"
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <random>
#include <memory>
#include "imgui.h"

class UIManager {
private:
    std::vector<float> ph_history;
    std::vector<float> kh_history;
    std::vector<float> nh4_history;
    std::vector<float> no2_history;
    std::vector<float> no3_history;
    std::vector<float> po4_history;
    std::vector<float> fe_history;

    int water_change_percent = 30;
    int topoff_water_type = 0;
    int wc_water_type = 0;
    float topoff_liters = 1.0f;
    float drain_liters = 1.0f;
    float fertilizer_amount_ml = 1.0f;
    float ui_scale = 1.4f;
    bool is_paused = false;
    int selected_tank_idx = 0;

    struct Entity2D {
        float x, y, vx, vy;
        bool is_shrimp;
        bool forms_school;
        float behavior_timer = 0.0f;
        bool wandering_alone = false;
    };

    std::vector<Entity2D> entities;

    // NEU: Nur noch relative X-Koordinate (0.0 bis 1.0), keine absoluten Pixel mehr!
    struct PlantEntity2D {
        float rel_x;
    };

    std::vector<PlantEntity2D> plant_entities;

    bool entities_initialized = false;
    int inspected_animal_idx = -1;
    int inspected_plant_idx = -1;

    std::mt19937 rng{std::random_device{}()};

public:
    void initHistory(const Aquarium &tank) { updateHistory(tank); }
    bool isPaused() const { return is_paused; }

    void updateHistory(const Aquarium &tank) {
        ph_history.push_back(static_cast<float>(tank.water.ph));
        kh_history.push_back(static_cast<float>(tank.water.kh));
        nh4_history.push_back(static_cast<float>(tank.water.nh4));
        no2_history.push_back(static_cast<float>(tank.water.no2));
        no3_history.push_back(static_cast<float>(tank.water.no3));
        po4_history.push_back(static_cast<float>(tank.water.po4));
        fe_history.push_back(static_cast<float>(tank.water.fe));

        auto capSize = [](std::vector<float> &vec) { if (vec.size() > 200) vec.erase(vec.begin()); };
        capSize(ph_history);
        capSize(kh_history);
        capSize(nh4_history);
        capSize(no2_history);
        capSize(no3_history);
        capSize(po4_history);
        capSize(fe_history);
    }

    void render(Aquarium &tank, int tickCounter) {
        ImGui::GetIO().FontGlobalScale = ui_scale;

        ImGui::Begin("Aquarium Simulation Dashboard");

        ImGui::SliderFloat("UI Scale (1440p Zoom)", &ui_scale, 1.0f, 2.5f, "%.1f");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Checkbox("Pause", &is_paused);
        ImGui::SameLine();
        ImGui::Text("   |   Simulated Time: Day %d, Hour %d", tickCounter / 24, tickCounter % 24);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // --- TANK AUSWAHL ---
        ImGui::SeparatorText("Aquarium Configuration");
        const char *tank_models[] = {
            "Dennerle Nano Cube 30L", "20 Gallon Breeder Tank (High)", "20 Gallon Breeder Tank (Long)"
        };
        if (ImGui::Combo("Select Tank Model", &selected_tank_idx, tank_models, IM_ARRAYSIZE(tank_models))) {
            if (selected_tank_idx == 0) tank.setTankModel(TankModel::dennerleNanoCube30());
            if (selected_tank_idx == 1) tank.setTankModel(TankModel::BreederTank_20G_High());
            if (selected_tank_idx == 2) tank.setTankModel(TankModel::BreederTank_20G_Long());
        }
        ImGui::Text("Net Water Volume: %.2f L / Max: %.2f L", tank.volume_liters, tank.max_capacity_liters);
        ImGui::Spacing();

        std::map<std::string, int> animal_counts;
        for (const auto &a: tank.livestock) animal_counts[a->common_name]++;
        ImGui::Text("Animals: %zu", tank.livestock.size());
        for (const auto &[name, count]: animal_counts) ImGui::Text("   - %dx %s", count, name.c_str());

        ImGui::Spacing();
        std::map<std::string, int> plant_counts;
        for (const auto &p: tank.flora) plant_counts[p->common_name]++;
        ImGui::Text("Plants: %zu", tank.flora.size());
        for (const auto &[name, count]: plant_counts) ImGui::Text("   - %dx %s", count, name.c_str());

        ImGui::Spacing();
        ImGui::SeparatorText("Water Parameters");
        ImGui::Spacing();
        ImGui::Text("pH: %.2f   |   KH: %.2f dKH", tank.water.ph, tank.water.kh);
        ImGui::Text("NH4: %.4f mg/L   |   NH3: %.4f mg/L", tank.water.nh4, tank.water.nh3);
        ImGui::Text("NO2: %.4f mg/L   |   NO3: %.4f mg/L", tank.water.no2, tank.water.no3);
        ImGui::Text("PO4: %.4f mg/L   |   Fe:  %.4f mg/L", tank.water.po4, tank.water.fe);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Water Parameter Graphs", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Spacing();
            if (!nh4_history.empty()) {
                int history_size = static_cast<int>(nh4_history.size());
                ImGui::PlotLines("pH", ph_history.data(), history_size, 0, nullptr, 5.0f, 10.0f, ImVec2(0, 45));
                ImGui::PlotLines("KH", kh_history.data(), history_size, 0, nullptr, 0.0f, 20.0f, ImVec2(0, 45));
                ImGui::PlotLines("NH4", nh4_history.data(), history_size, 0, nullptr, 0.0f, 2.0f, ImVec2(0, 45));
                ImGui::PlotLines("NO2", no2_history.data(), history_size, 0, nullptr, 0.0f, 2.0f, ImVec2(0, 45));
                ImGui::PlotLines("NO3", no3_history.data(), history_size, 0, nullptr, 0.0f, 30.0f, ImVec2(0, 45));
                ImGui::PlotLines("PO4", po4_history.data(), history_size, 0, nullptr, 0.0f, 2.0f, ImVec2(0, 45));
                ImGui::PlotLines("Fe", fe_history.data(), history_size, 0, nullptr, 0.0f, 1.0f, ImVec2(0, 45));
            }
            ImGui::Spacing();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Debug & Chemistry Inspector")) {
            ImGui::Spacing();
            ImGui::Text("--- Bacterial Populations ---");
            ImGui::Text("Nitrosomonas: %.6f", tank.chemistry.nitrosomonas_bacteria);
            ImGui::Text("Nitrobacter:  %.6f", tank.chemistry.nitrobacter_bacteria);
            ImGui::Text("Bio-Capacity Limit: %.4f", tank.biological_capacity);

            ImGui::Separator();
            ImGui::Text("--- Rates & Fluxes ---");
            double total_nh4_prod = 0.0;
            for (const auto &a: tank.livestock) total_nh4_prod += a->nh4_production_mg_per_hour;

            double total_n_cons = 0.0;
            for (const auto &p: tank.flora) total_n_cons += p->nitrogen_consumption_mg_per_hour;

            ImGui::Text("Animal NH4 Production: %.5f mg/h", total_nh4_prod);
            ImGui::Text("Plant Nitrogen Demand: %.5f mg/h", total_n_cons);
            ImGui::Text("Substrate Height: %.1f cm", tank.substrate.height_cm);
            ImGui::Spacing();
        }

        ImGui::Spacing();
        ImGui::SeparatorText("Add Organisms");
        ImGui::Spacing();
        if (ImGui::Button("Add Neocaridina")) tank.addAnimal(OrganismFactory::createNeocaridina());
        ImGui::SameLine();
        if (ImGui::Button("Add Neon Tetra")) tank.addAnimal(OrganismFactory::createNeonTetra());

        if (ImGui::Button("Hornwort")) tank.addPlant(OrganismFactory::createHornwortStem());
        ImGui::SameLine();
        if (ImGui::Button("Anubias")) tank.addPlant(OrganismFactory::createAnubias());
        ImGui::SameLine();
        if (ImGui::Button("Hygrophila")) tank.addPlant(OrganismFactory::createHygrophila());

        if (ImGui::Button("Rotala Orange")) tank.addPlant(OrganismFactory::createRotala());
        ImGui::SameLine();
        if (ImGui::Button("Moss Ball")) tank.addPlant(OrganismFactory::createMossBall());
        ImGui::SameLine();
        if (ImGui::Button("Cryptocoryne")) tank.addPlant(OrganismFactory::createCryptocoryne());

        ImGui::Spacing();
        ImGui::SeparatorText("Maintenance & Water Management");
        ImGui::Spacing();

        ImGui::Combo("Water Source", &wc_water_type, "Tap Water (22C)\0Osmosis Water\0");
        ImGui::SliderInt("##wc_slider", &water_change_percent, 10, 90, "Water Change: %d %%");
        ImGui::SameLine();
        if (ImGui::Button("Execute Water Change")) {
            WaterParameters source = (wc_water_type == 0)
                                         ? WaterParameters::createAverageTapWater(22.0)
                                         : WaterParameters::createOsmosisWater();
            tank.performWaterChange(static_cast<double>(water_change_percent), source);
        }

        ImGui::Spacing();
        ImGui::Combo("Top-off Source", &topoff_water_type, "Tap Water (22C)\0Osmosis Water\0");
        ImGui::InputFloat("Liters##topoff", &topoff_liters, 0.1f, 1.0f, "%.1f");
        ImGui::SameLine();
        if (ImGui::Button("Top Off Water")) {
            WaterParameters source = (topoff_water_type == 0)
                                         ? WaterParameters::createAverageTapWater(22.0)
                                         : WaterParameters::createOsmosisWater();
            tank.topOffWater(static_cast<double>(topoff_liters), source);
        }
        ImGui::SameLine();
        ImGui::InputFloat("Liters##drain", &drain_liters, 0.1f, 1.0f, "%.1f");
        ImGui::SameLine();
        if (ImGui::Button("Drain Water")) {
            tank.drainWater(static_cast<double>(drain_liters));
        }

        ImGui::Spacing();
        ImGui::SeparatorText("Fertilizers");
        ImGui::Spacing();
        ImGui::InputFloat("ml##fert", &fertilizer_amount_ml, 0.1f, 1.0f, "%.1f");
        ImGui::SameLine();
        if (ImGui::Button("Add NPK")) {
            tank.addFertilizer(ItemFactory::createNPKFertilizer(), static_cast<double>(fertilizer_amount_ml));
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Iron")) {
            tank.addFertilizer(ItemFactory::createIronFertilizer(), static_cast<double>(fertilizer_amount_ml));
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Phosphate")) {
            tank.addFertilizer(ItemFactory::createPhosphateFertilizer(), static_cast<double>(fertilizer_amount_ml));
        }

        ImGui::End();

        render2DSimulation(tank);
    }

private:
    void render2DSimulation(Aquarium &tank) {
        ImGui::Begin("2D Aquarium View");
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
        if (canvas_sz.x < 200.0f) canvas_sz.x = 200.0f;
        if (canvas_sz.y < 200.0f) canvas_sz.y = 200.0f;

        float total_h = canvas_sz.y;
        float tank_h = total_h * 0.90f;
        float tank_w = tank_h * (tank.model.width_cm / tank.model.height_cm);
        if (tank_w > canvas_sz.x - 20.0f) {
            tank_w = canvas_sz.x - 20.0f;
            tank_h = tank_w * (tank.model.height_cm / tank.model.width_cm);
        }

        float tank_x = canvas_p0.x + (canvas_sz.x - tank_w) * 0.5f;
        float tank_y = canvas_p0.y + (total_h * 0.10f);
        ImVec2 tank_p0(tank_x, tank_y);
        ImVec2 tank_p1(tank_x + tank_w, tank_y + tank_h);
        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        float water_ratio = std::clamp(static_cast<float>(tank.volume_liters / tank.max_capacity_liters), 0.0f, 1.0f);
        float water_top_y = tank_p1.y - (tank_h * water_ratio);
        float substrate_h = tank_h * 0.18f;
        ImVec2 sub_p0(tank_p0.x, tank_p1.y - substrate_h);

        // Rendering Hintergrund & Glas
        draw_list->AddRectFilled(ImVec2(tank_x - 12.0f, tank_p1.y), ImVec2(tank_x + tank_w + 12.0f, tank_p1.y + 14.0f),
                                 IM_COL32(75, 45, 20, 255));
        draw_list->AddRectFilled(ImVec2(tank_p0.x, water_top_y), tank_p1, IM_COL32(35, 75, 110, 255));
        draw_list->AddRectFilled(sub_p0, tank_p1, IM_COL32(110, 80, 50, 255));
        draw_list->AddLine(tank_p0, ImVec2(tank_p0.x, tank_p1.y), IM_COL32(180, 200, 220, 180), 3.0f);
        draw_list->AddLine(ImVec2(tank_p1.x, tank_p0.y), tank_p1, IM_COL32(180, 200, 220, 180), 3.0f);
        draw_list->AddLine(ImVec2(tank_p0.x, tank_p1.y), tank_p1, IM_COL32(180, 200, 220, 180), 3.0f);

        bool clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        ImVec2 mouse_pos = ImGui::GetMousePos();

        // 1. Pflanzen rendern (Dynamisch skaliert mit echtem Wachstum, Emers-Form und Färbung)
        // ... (Die Initialisierung der plant_entities bleibt gleich) ...

        float cm_to_px = tank_h / tank.model.height_cm; // Pixelskalierung pro Zentimeter

        for (size_t i = 0; i < tank.flora.size(); ++i) {
            auto &p_ent = plant_entities[i];
            auto &plant = tank.flora[i];

            float px = tank_p0.x + (tank_w * p_ent.rel_x);
            float py = sub_p0.y; // Bodenpflanzen starten am Substrat
            float p_height_px = plant->current_height_cm * cm_to_px;

            ImVec2 p_min, p_max;

            // -- ROTALA ORANGE JUICE --
            if (plant->scientific_name.find("Rotala") != std::string::npos) {
                float end_y = py - p_height_px;
                draw_list->AddLine(ImVec2(px, py), ImVec2(px, end_y), IM_COL32(160, 100, 60, 255), 2.0f); // Stängel

                float spacing = 12.0f / plant->density_factor;
                // GEILWUCHS: Bei Nährstoffmangel riesige Internodien-Abstände!

                for (float y = py; y > end_y; y -= spacing) {
                    if (y < water_top_y) {
                        // EMERS: Kriechend, kleine runde dicke grüne Blätter, winzige Blüten
                        draw_list->AddCircleFilled(ImVec2(px - 6, y), 5.0f, IM_COL32(50, 140, 50, 255));
                        draw_list->AddCircleFilled(ImVec2(px + 6, y - 3), 5.0f, IM_COL32(40, 130, 40, 255));
                        if (static_cast<int>(y) % 25 < 5) draw_list->AddCircleFilled(
                            ImVec2(px, y - 5), 2.5f, IM_COL32(200, 130, 255, 255)); // Blüte
                    } else {
                        // SUBMERS: Feine nadelige Blätter. Farbe blendet von Grün nach Orange!
                        int r = 100 + (140 * plant->coloration_factor); // Grüner Mangel vs. sattes Orange
                        int g = 180 - (90 * plant->coloration_factor);
                        int b = 50;
                        draw_list->AddLine(ImVec2(px - 12, y), ImVec2(px + 12, y - 6), IM_COL32(r, g, b, 255), 2.0f);
                        draw_list->AddLine(ImVec2(px + 12, y), ImVec2(px - 12, y - 6), IM_COL32(r+20, g+10, b, 255),
                                           2.0f);
                    }
                }
                p_min = ImVec2(px - 20, end_y - 5);
                p_max = ImVec2(px + 20, py);
            }

            // -- HYGROPHILA (Indischer Wasserfreund) --
            else if (plant->scientific_name.find("Hygrophila") != std::string::npos) {
                float end_y = py - p_height_px;
                draw_list->AddLine(ImVec2(px, py), ImVec2(px, end_y), IM_COL32(140, 190, 100, 255), 3.0f);

                float spacing = 15.0f / plant->density_factor;
                for (float y = py; y > end_y; y -= spacing) {
                    if (y < water_top_y) {
                        // EMERS: Derbe, dunkelgrüne ovale Blätter
                        draw_list->AddEllipseFilled(ImVec2(px - 12, y), ImVec2(10.0f, 6.0f),
                                                    IM_COL32(30, 100, 30, 255));
                        draw_list->AddEllipseFilled(ImVec2(px + 12, y - 5), ImVec2(10.0f, 6.0f),
                                                    IM_COL32(25, 90, 25, 255));
                    } else {
                        // SUBMERS: Rosa Adern abhängig von Eisen und Licht (Coloration Factor)
                        int r = 140 + (60 * plant->coloration_factor);
                        int g = 190 - (80 * plant->coloration_factor);
                        int b = 100;
                        draw_list->AddEllipseFilled(ImVec2(px - 15, y), ImVec2(14.0f, 5.0f), IM_COL32(r, g, b, 255));
                        draw_list->AddEllipseFilled(ImVec2(px + 15, y - 5), ImVec2(14.0f, 5.0f),
                                                    IM_COL32(r-20, g-20, b, 255));
                    }
                }
                p_min = ImVec2(px - 25, end_y - 5);
                p_max = ImVec2(px + 25, py);
            }

            // -- HORNWORT (Schwimmt immer auf dem Wasser, nimmt keinen Emers-Schaden) --
            else if (plant->scientific_name.find("Ceratophyllum") != std::string::npos) {
                float hw_y = std::max(water_top_y + 6.0f, tank_p0.y + 12.0f);
                float hw_len = p_height_px; // Länge statt Höhe!
                float start_x = px - (hw_len / 2.0f);
                float end_x = px + (hw_len / 2.0f);

                float sway = std::sin(ImGui::GetTime() * 0.7f) * 4.0f;
                draw_list->AddLine(ImVec2(start_x + sway, hw_y), ImVec2(end_x + sway, hw_y),
                                   IM_COL32(120, 170, 90, 255), 2.0f);

                float spacing = std::max<float>(8.0f, 15.0f / plant->density_factor);
                // Geilwuchs bei Algen = sehr lichte Nadeln
                for (float nx = start_x; nx < end_x; nx += spacing) {
                    draw_list->AddLine(ImVec2(nx + sway, hw_y), ImVec2(nx - 4 + sway, hw_y + 15),
                                       IM_COL32(90, 220, 90, 255), 1.5f);
                    draw_list->AddLine(ImVec2(nx + sway, hw_y), ImVec2(nx + 4 + sway, hw_y + 12),
                                       IM_COL32(100, 230, 90, 255), 1.5f);
                }
                p_min = ImVec2(start_x - 5, hw_y - 5);
                p_max = ImVec2(end_x + 5, hw_y + 20);
            }

            // -- MOSS BALL (Moosball) --
            else if (plant->scientific_name.find("Aegagropila") != std::string::npos) {
                float radius = p_height_px / 2.0f; // Höhe ist hier Durchmesser
                draw_list->AddCircleFilled(ImVec2(px, py - radius), radius, IM_COL32(30, 90, 40, 255));
                p_min = ImVec2(px - radius, py - (radius * 2));
                p_max = ImVec2(px + radius, py);
            }

            // -- ANUBIAS NANA & CRYPTOCORYNE (Hier kürzer gefasst, skalieren nun auch mit p_height_px) --
            else if (plant->scientific_name.find("Anubias") != std::string::npos) {
                float h = p_height_px;
                draw_list->AddEllipseFilled(ImVec2(px, py), ImVec2(h * 1.5f, h * 0.4f), IM_COL32(80, 120, 60, 255));
                draw_list->AddTriangleFilled(ImVec2(px - h * 0.6f, py - 5), ImVec2(px - h * 0.2f, py - h * 2.0f),
                                             ImVec2(px + h * 0.2f, py - 5), IM_COL32(40, 110, 40, 255));
                draw_list->AddTriangleFilled(ImVec2(px - h * 0.2f, py - 5), ImVec2(px + h * 0.4f, py - h * 2.5f),
                                             ImVec2(px + h * 0.6f, py - 5), IM_COL32(50, 145, 50, 255));
                p_min = ImVec2(px - h, py - h * 3);
                p_max = ImVec2(px + h, py);
            }

            // Inspektor Klick-Logik bleibt gleich...

            if (clicked && mouse_pos.x >= p_min.x && mouse_pos.x <= p_max.x && mouse_pos.y >= p_min.y && mouse_pos.y <=
                p_max.y) {
                inspected_plant_idx = i;
                inspected_animal_idx = -1;
            }
            if (inspected_plant_idx == i) draw_list->AddRect(p_min, p_max, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
        }

        // 2. Tiere rendern & klicken
        if (!entities_initialized || entities.size() != tank.livestock.size()) {
            if (entities.size() < tank.livestock.size()) {
                for (size_t i = entities.size(); i < tank.livestock.size(); ++i) {
                    Entity2D e{};
                    e.x = tank_p0.x + 40.0f + static_cast<float>(rand() % static_cast<int>(tank_w - 80.0f));
                    e.is_shrimp = (tank.livestock[i]->common_name.find("Neocaridina") != std::string::npos);
                    e.forms_school = tank.livestock[i]->isSchooling();
                    if (e.is_shrimp) {
                        e.y = tank_p1.y - substrate_h - 10.0f;
                        e.vy = 0.0f;
                    } else {
                        e.y = water_top_y + 30.0f + static_cast<float>(rand() % 50);
                        e.vy = 0.2f;
                    }
                    std::uniform_real_distribution<float> dist_vx(-0.8f, 0.8f);
                    e.vx = dist_vx(rng);
                    entities.push_back(e);
                }
            } else if (entities.size() > tank.livestock.size()) {
                entities.resize(tank.livestock.size());
            }
            entities_initialized = true;
        }

        float time_sec = static_cast<float>(ImGui::GetTime());
        for (size_t i = 0; i < entities.size(); ++i) {
            auto &e = entities[i];
            ImVec2 a_min, a_max;

            if (e.is_shrimp) {
                if (rand() % 100 < 5) {
                    std::uniform_real_distribution<float> dist_vx(-0.6f, 0.6f);
                    e.vx = dist_vx(rng);
                }
                e.x += e.vx;
                e.y = tank_p1.y - substrate_h - 10.0f;

                a_min = ImVec2(e.x - 16.0f, e.y - 10.0f);
                a_max = ImVec2(e.x + 16.0f, e.y + 6.0f);
                draw_list->AddRectFilled(ImVec2(e.x - 14.0f, e.y - 7.0f), ImVec2(e.x + 14.0f, e.y + 4.0f),
                                         IM_COL32(235, 60, 60, 255));
                draw_list->AddLine(ImVec2(e.x + 14, e.y - 4), ImVec2(e.x + 22, e.y - 12), IM_COL32(235, 60, 60, 255),
                                   2.0f);
            } else {
                e.behavior_timer += 0.016f;
                if (e.behavior_timer > 4.0f + static_cast<float>(rand() % 4)) {
                    e.behavior_timer = 0.0f;
                    e.wandering_alone = (rand() % 100 < 35);
                }

                if (rand() % 140 < 4) {
                    std::uniform_real_distribution<float> dist_v(-0.4f, 0.4f);
                    e.vx += dist_v(rng);
                    e.vy += dist_v(rng);
                    e.vx = std::clamp(e.vx, -0.5f, 0.5f);
                    e.vy = std::clamp(e.vy, -0.25f, 0.25f);
                }

                if (e.forms_school && !e.wandering_alone) {
                    float sep_x = 0.0f, sep_y = 0.0f;
                    float align_vx = 0.0f, align_vy = 0.0f;
                    int school_count = 0;
                    for (size_t j = 0; j < entities.size(); ++j) {
                        if (i == j) continue;
                        const auto &other = entities[j];
                        if (other.is_shrimp || !other.forms_school || other.wandering_alone) continue;
                        float dx = other.x - e.x, dy = other.y - e.y;
                        float dist = std::sqrt(dx * dx + dy * dy);
                        if (dist < 250.0f) {
                            school_count++;
                            align_vx += other.vx;
                            align_vy += other.vy;
                            if (dist < 60.0f) {
                                sep_x -= dx / (dist + 0.1f);
                                sep_y -= dy / (dist + 0.1f);
                            }
                        }
                    }
                    if (school_count > 0) {
                        align_vx /= static_cast<float>(school_count);
                        align_vy /= static_cast<float>(school_count);
                        e.vx += (align_vx - e.vx) * 0.01f;
                        e.vy += (align_vy - e.vy) * 0.01f;
                        e.vx += sep_x * 0.04f;
                        e.vy += sep_y * 0.04f;
                    }
                }

                e.x += e.vx;
                e.y += e.vy;

                a_min = ImVec2(e.x - 20.0f, e.y - 10.0f);
                a_max = ImVec2(e.x + 20.0f, e.y + 10.0f);
                draw_list->AddRectFilled(ImVec2(e.x - 17.0f, e.y - 7.0f), ImVec2(e.x + 17.0f, e.y + 7.0f),
                                         IM_COL32(30, 130, 255, 255));
                draw_list->AddRectFilled(ImVec2(e.x + 6.0f, e.y - 7.0f), ImVec2(e.x + 19.0f, e.y + 7.0f),
                                         IM_COL32(225, 45, 45, 255));
            }

            // Sicheres Auto-Clamping innerhalb der Beckenmaße
            if (e.x < tank_p0.x + 25.0f) {
                e.x = tank_p0.x + 25.0f;
                e.vx = std::abs(e.vx);
            }
            if (e.x > tank_p1.x - 25.0f) {
                e.x = tank_p1.x - 25.0f;
                e.vx = -std::abs(e.vx);
            }
            float min_y = water_top_y + 25.0f;
            float max_y = tank_p1.y - substrate_h - 10.0f;
            if (e.y < min_y) {
                e.y = min_y;
                e.vy = std::abs(e.vy);
            }
            if (e.y > max_y) {
                e.y = max_y;
                e.vy = -std::abs(e.vy);
            }

            if (clicked && mouse_pos.x >= a_min.x && mouse_pos.x <= a_max.x && mouse_pos.y >= a_min.y && mouse_pos.y <=
                a_max.y) {
                inspected_animal_idx = i;
                inspected_plant_idx = -1;
            }
            if (inspected_animal_idx == i) draw_list->AddRect(a_min, a_max, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
        }

        // 3. Inspektor-Tooltip
        if (inspected_animal_idx >= 0 && inspected_animal_idx < tank.livestock.size()) {
            ImGui::SetNextWindowPos(ImVec2(canvas_p0.x + 15, canvas_p0.y + 15));
            ImGui::BeginChild("InspectorOverlay", ImVec2(300, 120), true,
                              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
            auto &a = tank.livestock[inspected_animal_idx];
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Inspecting: %s", a->common_name.c_str());
            ImGui::Text("Scientific: %s", a->scientific_name.c_str());
            ImGui::Text("HP: %.1f / 100", a->health_hp);
            ImGui::Text("Biomass: %.2f g", a->biomass_g);
            if (ImGui::Button("Close Inspector")) inspected_animal_idx = -1;
            ImGui::EndChild();
        } else if (inspected_plant_idx >= 0 && inspected_plant_idx < tank.flora.size()) {
            ImGui::SetNextWindowPos(ImVec2(canvas_p0.x + 15, canvas_p0.y + 15));
            ImGui::BeginChild("InspectorOverlay", ImVec2(300, 120), true,
                              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
            auto &p = tank.flora[inspected_plant_idx];
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Inspecting: %s", p->common_name.c_str());
            ImGui::Text("Scientific: %s", p->scientific_name.c_str());
            ImGui::Text("HP: %.1f / 100", p->health_hp);
            ImGui::Text("Biomass: %.2f g", p->biomass_g);
            if (ImGui::Button("Close Inspector")) inspected_plant_idx = -1;
            ImGui::EndChild();
        }

        ImGui::End();
    }
};

#endif //AQUARIUMSIM_UIMANAGER_H
