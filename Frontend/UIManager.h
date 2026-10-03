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
    std::vector<float> ph_hist, kh_hist, nh4_hist, no2_hist, no3_hist, po4_hist, fe_hist, k_hist, co2_hist, o2_hist;

    int water_change_percent = 30, topoff_water_type = 0, wc_water_type = 0;
    float topoff_liters = 1.0f, drain_liters = 1.0f, fertilizer_amount_ml = 1.0f, ui_scale = 1.4f;
    bool is_paused = false;
    int selected_tank_idx = 0, selected_graph_idx = -1;

    struct Entity2D { float x, y, vx, vy; bool is_shrimp, forms_school, wandering_alone; float behavior_timer = 0.0f; };
    std::vector<Entity2D> entities;
    struct PlantEntity2D { float rel_x; };
    std::vector<PlantEntity2D> plant_entities;

    bool entities_initialized = false;
    int inspected_animal_idx = -1, inspected_plant_idx = -1;
    std::mt19937 rng{std::random_device{}()};

    ImVec4 getDeltaColor(double delta, const std::string& type) {
        if (std::abs(delta) < 0.00005) return ImVec4(0.6f, 0.6f, 0.6f, 1.0f); // Neutral
        if (type == "TOXIC") return delta > 0 ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
        if (type == "NUTRIENT" || type == "O2") return delta > 0 ? ImVec4(0.3f, 1.0f, 0.3f, 1.0f) : ImVec4(1.0f, 0.6f, 0.2f, 1.0f);
        return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }

public:
    int speed_mode = 3; // Standard: 3 = 1h/s
    int simulation_speed_multiplier = 1; // 1 = 1h/s, 12 = 12h/s, 24 = 24h/s
    void initHistory(const Aquarium &tank) { updateHistory(tank); }
    bool isPaused() const { return is_paused; }

    void updateHistory(const Aquarium &tank) {
        ph_hist.push_back(static_cast<float>(tank.water.ph)); kh_hist.push_back(static_cast<float>(tank.water.kh));
        nh4_hist.push_back(static_cast<float>(tank.water.nh4)); no2_hist.push_back(static_cast<float>(tank.water.no2));
        no3_hist.push_back(static_cast<float>(tank.water.no3)); po4_hist.push_back(static_cast<float>(tank.water.po4));
        fe_hist.push_back(static_cast<float>(tank.water.fe)); k_hist.push_back(static_cast<float>(tank.water.k));
        co2_hist.push_back(static_cast<float>(tank.water.co2)); o2_hist.push_back(static_cast<float>(tank.water.o2));
        auto capSize = [](std::vector<float> &vec) { if (vec.size() > 200) vec.erase(vec.begin()); };
        capSize(ph_hist); capSize(kh_hist); capSize(nh4_hist); capSize(no2_hist); capSize(no3_hist);
        capSize(po4_hist); capSize(fe_hist); capSize(k_hist); capSize(co2_hist); capSize(o2_hist);
    }

    void render(Aquarium &tank, int tickCounter) {
        ImGui::GetIO().FontGlobalScale = ui_scale;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Aquarium Simulation Engine", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

        // --- GESCHWINDIGKEITS- UND STATUSLEISTE ---
        if (ImGui::Button(is_paused ? "Resume" : "Pause", ImVec2(75, 25))) is_paused = !is_paused;
        ImGui::SameLine();

        ImGui::Text("Speed:"); ImGui::SameLine();
        if (ImGui::Button("Echtzeit", ImVec2(65, 25))) speed_mode = 0; ImGui::SameLine();
        if (ImGui::Button("1min/s", ImVec2(55, 25))) speed_mode = 1; ImGui::SameLine();
        if (ImGui::Button("10min/s", ImVec2(65, 25))) speed_mode = 2; ImGui::SameLine();
        if (ImGui::Button("1h/s", ImVec2(45, 25))) speed_mode = 3; ImGui::SameLine();
        if (ImGui::Button("12h/s", ImVec2(50, 25))) speed_mode = 4; ImGui::SameLine();
        if (ImGui::Button("24h/s", ImVec2(50, 25))) speed_mode = 5;

        ImGui::SameLine(460);
        ImGui::SetNextItemWidth(100);
        ImGui::SliderFloat("UI Scale", &ui_scale, 1.0f, 2.5f, "%.1f");

        ImGui::SameLine(610);
        ImGui::Text("Simulated Time: Day %d, Hour %d", tickCounter / 24, tickCounter % 24);
        ImGui::Separator();

        // Left Panel (Scrollable Controls)
        float left_panel_w = ImGui::GetContentRegionAvail().x * 0.4f;
        ImGui::BeginChild("LeftPanel", ImVec2(left_panel_w, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        if (ImGui::BeginTabBar("MainTabs")) {

            // ================== TAB 1: DASHBOARD ==================
            if (ImGui::BeginTabItem("Dashboard & Chemistry")) {
                ImGui::Spacing();
                ImGui::Text("Net Water Volume: %.2f L / Max: %.2f L", tank.volume_liters, tank.max_capacity_liters);
                ImGui::Spacing(); ImGui::SeparatorText("Water Parameters (Click for Graph)");

                int cols = ImGui::GetContentRegionAvail().x > 400 ? 3 : 2;
                if (ImGui::BeginTable("WaterParamsTable", cols, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
                    auto RenderCell = [&](const char* label, double val, double delta, const char* unit, int graph_id, const std::string& type) {
                        ImGui::TableNextColumn();
                        char buf[64]; snprintf(buf, sizeof(buf), "%-4s : %6.2f %s", label, val, unit);
                        if (ImGui::Selectable(buf, selected_graph_idx == graph_id)) selected_graph_idx = (selected_graph_idx == graph_id) ? -1 : graph_id;
                        ImGui::SameLine();
                        ImVec4 col = getDeltaColor(delta, type);
                        ImGui::TextColored(col, "(%+6.3f)", delta);
                    };

                    RenderCell("pH", tank.water.ph, tank.water_deltas.ph, "", 0, "NEUTRAL");
                    RenderCell("KH", tank.water.kh, tank.water_deltas.kh, "dKH", 1, "NEUTRAL");
                    RenderCell("O2", tank.water.o2, tank.water_deltas.o2, "mg", 9, "O2");
                    RenderCell("CO2", tank.water.co2, tank.water_deltas.co2, "mg", 8, "NUTRIENT");
                    RenderCell("NH4", tank.water.nh4, tank.water_deltas.nh4, "mg", 2, "TOXIC");
                    RenderCell("NO2", tank.water.no2, tank.water_deltas.no2, "mg", 3, "TOXIC");
                    RenderCell("NO3", tank.water.no3, tank.water_deltas.no3, "mg", 4, "NUTRIENT");
                    RenderCell("PO4", tank.water.po4, tank.water_deltas.po4, "mg", 5, "NUTRIENT");
                    RenderCell("K", tank.water.k, tank.water_deltas.k, "mg", 7, "NUTRIENT");
                    RenderCell("Fe", tank.water.fe, tank.water_deltas.fe, "mg", 6, "NUTRIENT");
                    ImGui::EndTable();
                }

                if (selected_graph_idx != -1) {
                    ImGui::Spacing();
                    const float* data = nullptr; int size = 0; const char* title = "";
                    switch (selected_graph_idx) {
                        case 0: data = ph_hist.data(); size = ph_hist.size(); title = "pH Trend"; break;
                        case 1: data = kh_hist.data(); size = kh_hist.size(); title = "KH Trend"; break;
                        case 2: data = nh4_hist.data(); size = nh4_hist.size(); title = "NH4 Trend"; break;
                        case 3: data = no2_hist.data(); size = no2_hist.size(); title = "NO2 Trend"; break;
                        case 4: data = no3_hist.data(); size = no3_hist.size(); title = "NO3 Trend"; break;
                        case 5: data = po4_hist.data(); size = po4_hist.size(); title = "PO4 Trend"; break;
                        case 6: data = fe_hist.data(); size = fe_hist.size(); title = "Fe Trend"; break;
                        case 7: data = k_hist.data(); size = k_hist.size(); title = "K Trend"; break;
                        case 8: data = co2_hist.data(); size = co2_hist.size(); title = "CO2 Trend"; break;
                        case 9: data = o2_hist.data(); size = o2_hist.size(); title = "O2 Trend"; break;
                    }
                    if (size > 0) {
                        float v_min = data[0], v_max = data[0];
                        for(int i=1; i<size; i++) { if (data[i] < v_min) v_min = data[i]; if (data[i] > v_max) v_max = data[i]; }
                        float pad = (v_max - v_min) * 0.15f; if (pad == 0.0f) pad = (v_min > 0.0f) ? v_min * 0.1f : 0.1f;
                        ImGui::PlotLines(title, data, size, 0, nullptr, v_min - pad, v_max + pad, ImVec2(0, 100));
                    }
                }

                ImGui::Spacing(); ImGui::SeparatorText("Detailed System Fluxes");
                double a_nh4_prod = 0, a_co2_prod = 0, a_o2_cons = 0;
                for (const auto &a: tank.livestock) { a_nh4_prod += a->nh4_production_mg_per_hour; a_co2_prod += a->co2_production_mg_per_hour; a_o2_cons += a->o2_consumption_mg_per_hour; }
                double p_n_cons = 0, p_p_cons = 0, p_fe_cons = 0, p_k_cons = 0, p_co2_cons = 0, p_o2_prod = 0;
                for (const auto &p: tank.flora) { p_n_cons += p->nitrogen_consumption_mg_per_hour; p_p_cons += p->po4_consumption_mg_per_hour; p_fe_cons += p->fe_consumption_mg_per_hour; p_k_cons += p->k_consumption_mg_per_hour; p_co2_cons += p->co2_consumption_mg_per_hour; p_o2_prod += p->o2_production_mg_per_hour; }

                ImGui::TextColored(ImVec4(0.7,0.7,1,1), "O2:  +%.4f (Plants) | -%.4f (Animals)", tank.is_day ? p_o2_prod : 0.0, a_o2_cons);
                ImGui::TextColored(ImVec4(0.7,1,0.7,1), "CO2: +%.4f (Animals) | -%.4f (Plants)", a_co2_prod, tank.is_day ? p_co2_cons : 0.0);
                ImGui::TextColored(ImVec4(1,0.7,0.7,1), "NH4: +%.4f (Animals) | -%.4f (Filter/Plants)", a_nh4_prod, tank.water_deltas.nh4 * tank.volume_liters);
                ImGui::EndTabItem();
            }

            // ================== TAB 2: ECOSYSTEM ==================
            if (ImGui::BeginTabItem("Ecosystem Setup")) {
                ImGui::Spacing(); ImGui::SeparatorText("Tank Model");
                const char *tank_models[] = { "Dennerle Nano Cube 30L", "20 Gallon Breeder Tank (High)", "20 Gallon Breeder Tank (Long)" };
                if (ImGui::Combo("Select Tank", &selected_tank_idx, tank_models, IM_ARRAYSIZE(tank_models))) {
                    if (selected_tank_idx == 0) tank.setTankModel(TankModel::dennerleNanoCube30());
                    if (selected_tank_idx == 1) tank.setTankModel(TankModel::BreederTank_20G_High());
                    if (selected_tank_idx == 2) tank.setTankModel(TankModel::BreederTank_20G_Long());
                    entities_initialized = false;
                }

                ImGui::Spacing(); ImGui::SeparatorText("Add Livestock");
                if (ImGui::Button("Add Neocaridina Shrimp", ImVec2(180, 30))) tank.addAnimal(OrganismFactory::createNeocaridina()); ImGui::SameLine();
                if (ImGui::Button("Add Neon Tetra", ImVec2(180, 30))) tank.addAnimal(OrganismFactory::createNeonTetra());

                ImGui::Spacing(); ImGui::SeparatorText("Add Flora");
                if (ImGui::Button("Hornwort", ImVec2(120, 30))) tank.addPlant(OrganismFactory::createHornwortStem()); ImGui::SameLine();
                if (ImGui::Button("Anubias Nana", ImVec2(120, 30))) tank.addPlant(OrganismFactory::createAnubias()); ImGui::SameLine();
                if (ImGui::Button("Hygrophila", ImVec2(120, 30))) tank.addPlant(OrganismFactory::createHygrophila());
                if (ImGui::Button("Rotala Orange", ImVec2(120, 30))) tank.addPlant(OrganismFactory::createRotala()); ImGui::SameLine();
                if (ImGui::Button("Moss Ball", ImVec2(120, 30))) tank.addPlant(OrganismFactory::createMossBall()); ImGui::SameLine();
                if (ImGui::Button("Cryptocoryne", ImVec2(120, 30))) tank.addPlant(OrganismFactory::createCryptocoryne());

                ImGui::Spacing();
                std::map<std::string, int> pop;
                for (const auto &a: tank.livestock) pop[a->common_name]++;
                for (const auto &p: tank.flora) pop[p->common_name]++;
                ImGui::SeparatorText("Current Population");
                for (const auto &[name, count]: pop) ImGui::BulletText("%dx %s", count, name.c_str());

                ImGui::EndTabItem();
            }

            // ================== TAB 3: EQUIPMENT ==================
            if (ImGui::BeginTabItem("Equipment")) {
                ImGui::Spacing(); ImGui::SeparatorText("Lighting System");
                ImGui::SliderFloat("Light Intensity", &tank.light_intensity, 0.0f, 1.0f, "%.2f");
                const char *spectrums[] = { "Full Spectrum (White)", "RGB (Plant Color)", "Cold White", "Warm White" };
                ImGui::Combo("Light Spectrum", &tank.light_spectrum, spectrums, IM_ARRAYSIZE(spectrums));

                ImGui::Spacing(); ImGui::SeparatorText("CO2 Injection");
                ImGui::Checkbox("CO2 Active", &tank.co2_active); ImGui::SameLine();
                ImGui::Checkbox("Night Shutoff", &tank.co2_night_shutoff);
                ImGui::SliderFloat("Flow (BPS)", &tank.co2_bps, 0.1f, 5.0f, "%.1f Bubbles/Sec");

                ImGui::EndTabItem();
            }

            // ================== TAB 4: MAINTENANCE ==================
            if (ImGui::BeginTabItem("Maintenance")) {
                ImGui::Spacing(); ImGui::SeparatorText("Water Change");
                ImGui::Combo("Source##wc", &wc_water_type, "Tap Water (22C)\0Osmosis Water\0");
                ImGui::SliderInt("Amount", &water_change_percent, 10, 90, "%d %%");
                if (ImGui::Button("Execute Water Change", ImVec2(-1, 35))) {
                    WaterParameters src = (wc_water_type == 0) ? WaterParameters::createAverageTapWater(22.0) : WaterParameters::createOsmosisWater();
                    tank.performWaterChange(static_cast<double>(water_change_percent), src);
                }

                ImGui::Spacing(); ImGui::SeparatorText("Drain / Top-off");
                ImGui::Combo("Source##top", &topoff_water_type, "Tap Water (22C)\0Osmosis Water\0");
                ImGui::InputFloat("Liters##dt", &topoff_liters, 0.1f, 1.0f, "%.1f");
                if (ImGui::Button("Top Off Water", ImVec2(150, 30))) {
                    WaterParameters src = (topoff_water_type == 0) ? WaterParameters::createAverageTapWater(22.0) : WaterParameters::createOsmosisWater();
                    tank.topOffWater(static_cast<double>(topoff_liters), src);
                }
                ImGui::SameLine();
                if (ImGui::Button("Drain Water", ImVec2(150, 30))) tank.drainWater(static_cast<double>(topoff_liters));

                ImGui::Spacing(); ImGui::SeparatorText("Fertilizer Dosing");
                ImGui::InputFloat("Dose (ml)", &fertilizer_amount_ml, 0.1f, 1.0f, "%.1f");
                if (ImGui::Button("Add NPK (Makros)", ImVec2(120, 30))) tank.addFertilizer(ItemFactory::createNPKFertilizer(), fertilizer_amount_ml); ImGui::SameLine();
                if (ImGui::Button("Add Iron (Fe)", ImVec2(120, 30))) tank.addFertilizer(ItemFactory::createIronFertilizer(), fertilizer_amount_ml); ImGui::SameLine();
                if (ImGui::Button("Add Phosphate", ImVec2(120, 30))) tank.addFertilizer(ItemFactory::createPhosphateFertilizer(), fertilizer_amount_ml);

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Right Panel (Render 2D View)
        ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
        render2DSimulation(tank);
        ImGui::EndChild();

        ImGui::End();
    }

private:
    void render2DSimulation(Aquarium &tank) {
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
        if (canvas_sz.x < 200.0f) canvas_sz.x = 200.0f;
        if (canvas_sz.y < 200.0f) canvas_sz.y = 200.0f;

        float total_h = canvas_sz.y - 60.0f;
        float tank_h = total_h;
        float tank_w = tank_h * (tank.model.width_cm / tank.model.height_cm);
        if (tank_w > canvas_sz.x - 40.0f) { tank_w = canvas_sz.x - 40.0f; tank_h = tank_w * (tank.model.height_cm / tank.model.width_cm); }
        float tank_x = canvas_p0.x + (canvas_sz.x - tank_w) * 0.5f;
        float tank_y = canvas_p0.y + canvas_sz.y - 40.0f - tank_h;
        ImVec2 tank_p0(tank_x, tank_y), tank_p1(tank_x + tank_w, tank_y + tank_h);
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        float cm_to_px = tank_h / tank.model.height_cm;

        draw_list->AddRectFilled(ImVec2(tank_p0.x - 2.0f*cm_to_px, tank_p1.y), ImVec2(tank_p1.x + 2.0f*cm_to_px, tank_p1.y + 2.0f*cm_to_px), IM_COL32(35, 35, 35, 255));

        float water_ratio = std::clamp(static_cast<float>(tank.volume_liters / tank.max_capacity_liters), 0.0f, 1.0f);
        float water_top_y = tank_p1.y - (tank_h * water_ratio);
        float substrate_h = tank_h * (tank.substrate.height_cm / tank.model.height_cm);
        ImVec2 sub_p0(tank_p0.x, tank_p1.y - substrate_h);

        float active_l = tank.is_day ? tank.light_intensity : 0.05f;
        int wr = 35, wg = 75, wb = 110;
        if (tank.is_day) {
            if (tank.light_spectrum == 0) { wr = 60; wg = 120; wb = 160; }
            else if (tank.light_spectrum == 1) { wr = 80; wg = 100; wb = 150; }
            else if (tank.light_spectrum == 2) { wr = 40; wg = 130; wb = 180; }
            else if (tank.light_spectrum == 3) { wr = 90; wg = 130; wb = 120; }
            wr = static_cast<int>(wr * active_l); wg = static_cast<int>(wg * active_l); wb = static_cast<int>(wb * active_l);
        }
        draw_list->AddRectFilled(ImVec2(tank_p0.x, water_top_y), tank_p1, IM_COL32(wr, wg, wb, 255));
        draw_list->AddRectFilled(sub_p0, tank_p1, IM_COL32(110, 80, 50, 255));
        draw_list->AddLine(tank_p0, ImVec2(tank_p0.x, tank_p1.y), IM_COL32(180, 200, 220, 180), 3.0f);
        draw_list->AddLine(ImVec2(tank_p1.x, tank_p0.y), tank_p1, IM_COL32(180, 200, 220, 180), 3.0f);
        draw_list->AddLine(ImVec2(tank_p0.x, tank_p1.y), tank_p1, IM_COL32(180, 200, 220, 180), 3.0f);

        bool clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        ImVec2 mouse_pos = ImGui::GetMousePos();

        if (plant_entities.size() < tank.flora.size()) {
            std::uniform_real_distribution<float> dist_rel_x(0.1f, 0.9f);
            for (size_t i = plant_entities.size(); i < tank.flora.size(); ++i) {
                PlantEntity2D p; p.rel_x = dist_rel_x(rng); plant_entities.push_back(p);
            }
        } else if (plant_entities.size() > tank.flora.size()) plant_entities.resize(tank.flora.size());

        for (size_t i = 0; i < tank.flora.size(); ++i) {
            auto &p_ent = plant_entities[i]; auto &plant = tank.flora[i];
            if (!plant) continue;

            float px = tank_p0.x + (tank_w * p_ent.rel_x);
            float py = sub_p0.y, p_height_px = plant->current_height_cm * cm_to_px;
            ImVec2 p_min, p_max;

            // ROTALA
            if (plant->scientific_name.find("Rotala") != std::string::npos) {
                float end_y = py - p_height_px; draw_list->AddLine(ImVec2(px, py), ImVec2(px, end_y), IM_COL32(160, 100, 60, 255), 2.0f);
                float spacing = 12.0f / plant->density_factor;
                for (float y = py; y > end_y; y -= spacing) {
                    if (y < water_top_y) { draw_list->AddCircleFilled(ImVec2(px - 6, y), 5.0f, IM_COL32(50, 140, 50, 255)); draw_list->AddCircleFilled(ImVec2(px + 6, y - 3), 5.0f, IM_COL32(40, 130, 40, 255)); }
                    else { int r = 100 + (140 * plant->coloration_factor), g = 180 - (90 * plant->coloration_factor), b = 50; draw_list->AddLine(ImVec2(px - 12, y), ImVec2(px + 12, y - 6), IM_COL32(r, g, b, 255), 2.0f); draw_list->AddLine(ImVec2(px + 12, y), ImVec2(px - 12, y - 6), IM_COL32(r+20, g+10, b, 255), 2.0f); }
                }
                p_min = ImVec2(px - 20, end_y - 5); p_max = ImVec2(px + 20, py);
            }
            // HYGROPHILA
            else if (plant->scientific_name.find("Hygrophila") != std::string::npos) {
                float end_y = py - p_height_px; draw_list->AddLine(ImVec2(px, py), ImVec2(px, end_y), IM_COL32(140, 190, 100, 255), 3.0f);
                float spacing = 15.0f / plant->density_factor;
                for (float y = py; y > end_y; y -= spacing) {
                    if (y < water_top_y) { draw_list->AddEllipseFilled(ImVec2(px - 12, y), ImVec2(10.0f, 6.0f), IM_COL32(30, 100, 30, 255)); draw_list->AddEllipseFilled(ImVec2(px + 12, y - 5), ImVec2(10.0f, 6.0f), IM_COL32(25, 90, 25, 255)); }
                    else { int r = 140 + (60 * plant->coloration_factor), g = 190 - (80 * plant->coloration_factor), b = 100; draw_list->AddEllipseFilled(ImVec2(px - 15, y), ImVec2(14.0f, 5.0f), IM_COL32(r, g, b, 255)); draw_list->AddEllipseFilled(ImVec2(px + 15, y - 5), ImVec2(14.0f, 5.0f), IM_COL32(r-20, g-20, b, 255)); }
                }
                p_min = ImVec2(px - 25, end_y - 5); p_max = ImVec2(px + 25, py);
            }
            // HORNKRAUT
            else if (plant->scientific_name.find("Ceratophyllum") != std::string::npos) {
                float hw_y = std::max(water_top_y + 6.0f, tank_p0.y + 12.0f); float hw_len = std::min(p_height_px, tank_w - 20.0f);
                float start_x = px - (hw_len / 2.0f); if (start_x < tank_p0.x + 10.0f) start_x = tank_p0.x + 10.0f;
                float end_x = start_x + hw_len; if (end_x > tank_p1.x - 10.0f) { end_x = tank_p1.x - 10.0f; start_x = end_x - hw_len; }
                float sway = std::sin(ImGui::GetTime() * 0.7f) * 4.0f;
                draw_list->AddLine(ImVec2(start_x + sway, hw_y), ImVec2(end_x + sway, hw_y), IM_COL32(120, 170, 90, 255), 2.0f);
                float spacing = std::max<float>(8.0f, 15.0f / plant->density_factor);
                for (float nx = start_x; nx < end_x; nx += spacing) { draw_list->AddLine(ImVec2(nx + sway, hw_y), ImVec2(nx - 4 + sway, hw_y + 15), IM_COL32(90, 220, 90, 255), 1.5f); draw_list->AddLine(ImVec2(nx + sway, hw_y), ImVec2(nx + 4 + sway, hw_y + 12), IM_COL32(100, 230, 90, 255), 1.5f); }
                p_min = ImVec2(start_x - 5, hw_y - 5); p_max = ImVec2(end_x + 5, hw_y + 20);
            }
            // MOOSBALL
            else if (plant->scientific_name.find("Aegagropila") != std::string::npos) {
                float radius = p_height_px / 2.0f; draw_list->AddCircleFilled(ImVec2(px, py - radius), radius, IM_COL32(30, 90, 40, 255)); p_min = ImVec2(px - radius, py - (radius * 2)); p_max = ImVec2(px + radius, py);
            }
            // ANUBIAS & CRYPTOCORYNE
            else if (plant->scientific_name.find("Anubias") != std::string::npos || plant->scientific_name.find("Cryptocoryne") != std::string::npos) {
                float h = p_height_px; draw_list->AddEllipseFilled(ImVec2(px, py), ImVec2(h * 1.5f, h * 0.4f), IM_COL32(80, 120, 60, 255));
                draw_list->AddTriangleFilled(ImVec2(px - h * 0.6f, py - 5), ImVec2(px - h * 0.2f, py - h * 2.0f), ImVec2(px + h * 0.2f, py - 5), IM_COL32(40, 110, 40, 255));
                draw_list->AddTriangleFilled(ImVec2(px - h * 0.2f, py - 5), ImVec2(px + h * 0.4f, py - h * 2.5f), ImVec2(px + h * 0.6f, py - 5), IM_COL32(50, 145, 50, 255));
                p_min = ImVec2(px - h, py - h * 3); p_max = ImVec2(px + h, py);
            }

            if (clicked && mouse_pos.x >= p_min.x && mouse_pos.x <= p_max.x && mouse_pos.y >= p_min.y && mouse_pos.y <= p_max.y) { inspected_plant_idx = i; inspected_animal_idx = -1; }
            if (inspected_plant_idx == i) draw_list->AddRect(p_min, p_max, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
        }

        // 2. Tiere
        if (!entities_initialized || entities.size() != tank.livestock.size()) {
            entities.resize(tank.livestock.size());
            for (size_t i = 0; i < tank.livestock.size(); ++i) {
                if (!tank.livestock[i]) continue;
                entities[i].x = tank_p0.x + 40.0f + static_cast<float>(rand() % static_cast<int>(tank_w - 80.0f));
                entities[i].is_shrimp = (tank.livestock[i]->common_name.find("Neocaridina") != std::string::npos);
                entities[i].forms_school = tank.livestock[i]->isSchooling();
                if (entities[i].is_shrimp) entities[i].y = tank_p1.y - substrate_h - 10.0f;
                else entities[i].y = water_top_y + 30.0f + static_cast<float>(rand() % 50);
            }
            entities_initialized = true;
        }

        for (size_t i = 0; i < entities.size(); ++i) {
            auto &e = entities[i]; ImVec2 a_min, a_max; auto &animal = tank.livestock[i]; if (!animal) continue;
            if (animal->is_suffocating && !e.is_shrimp) { e.vy = -1.0f; if (e.y <= water_top_y + 15.0f) e.vy = 0.0f; }

            if (e.is_shrimp) {
                if (rand() % 100 < 5) { std::uniform_real_distribution<float> dist_vx(-0.6f, 0.6f); e.vx = dist_vx(rng); }
                e.x += e.vx; e.y = tank_p1.y - substrate_h - 10.0f;
                a_min = ImVec2(e.x - 16.0f, e.y - 10.0f); a_max = ImVec2(e.x + 16.0f, e.y + 6.0f);
                draw_list->AddRectFilled(ImVec2(e.x - 14.0f, e.y - 7.0f), ImVec2(e.x + 14.0f, e.y + 4.0f), IM_COL32(235, 60, 60, 255));
                draw_list->AddLine(ImVec2(e.x + 14, e.y - 4), ImVec2(e.x + 22, e.y - 12), IM_COL32(235, 60, 60, 255), 2.0f);
            } else {
                if (!animal->is_suffocating) {
                    e.behavior_timer += 0.016f;
                    if (e.behavior_timer > 4.0f + static_cast<float>(rand() % 4)) { e.behavior_timer = 0.0f; e.wandering_alone = (rand() % 100 < 35); }
                    if (rand() % 140 < 4) { std::uniform_real_distribution<float> dist_v(-0.4f, 0.4f); e.vx += dist_v(rng); e.vy += dist_v(rng); e.vx = std::clamp(e.vx, -0.5f, 0.5f); e.vy = std::clamp(e.vy, -0.25f, 0.25f); }
                    if (e.forms_school && !e.wandering_alone) {
                        float sep_x = 0.0f, sep_y = 0.0f, align_vx = 0.0f, align_vy = 0.0f; int school_count = 0;
                        for (size_t j = 0; j < entities.size(); ++j) {
                            if (i == j || entities[j].is_shrimp || !entities[j].forms_school || entities[j].wandering_alone) continue;
                            float dx = entities[j].x - e.x, dy = entities[j].y - e.y, dist = std::sqrt(dx * dx + dy * dy);
                            if (dist < 250.0f) { school_count++; align_vx += entities[j].vx; align_vy += entities[j].vy; if (dist < 60.0f) { sep_x -= dx / (dist + 0.1f); sep_y -= dy / (dist + 0.1f); } }
                        }
                        if (school_count > 0) { e.vx += ((align_vx/school_count) - e.vx) * 0.01f; e.vy += ((align_vy/school_count) - e.vy) * 0.01f; e.vx += sep_x * 0.04f; e.vy += sep_y * 0.04f; }
                    }
                }
                e.x += e.vx; e.y += e.vy;
                a_min = ImVec2(e.x - 20.0f, e.y - 10.0f); a_max = ImVec2(e.x + 20.0f, e.y + 10.0f);
                draw_list->AddRectFilled(ImVec2(e.x - 17.0f, e.y - 7.0f), ImVec2(e.x + 17.0f, e.y + 7.0f), IM_COL32(30, 130, 255, 255));
                draw_list->AddRectFilled(ImVec2(e.x + 6.0f, e.y - 7.0f), ImVec2(e.x + 19.0f, e.y + 7.0f), IM_COL32(225, 45, 45, 255));
            }

            if (e.x < tank_p0.x + 25.0f) { e.x = tank_p0.x + 25.0f; e.vx = std::abs(e.vx); }
            if (e.x > tank_p1.x - 25.0f) { e.x = tank_p1.x - 25.0f; e.vx = -std::abs(e.vx); }
            float min_y = water_top_y + 10.0f, max_y = tank_p1.y - substrate_h - 10.0f;
            if (e.y < min_y) { e.y = min_y; e.vy = std::abs(e.vy); }
            if (e.y > max_y) { e.y = max_y; e.vy = -std::abs(e.vy); }

            if (clicked && mouse_pos.x >= a_min.x && mouse_pos.x <= a_max.x && mouse_pos.y >= a_min.y && mouse_pos.y <= a_max.y) { inspected_animal_idx = i; inspected_plant_idx = -1; }
            if (inspected_animal_idx == i) draw_list->AddRect(a_min, a_max, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
        }

        // 3. Inspektor-Tooltip (Mit Warnungen & Details)
        if (inspected_animal_idx >= 0 && inspected_animal_idx < tank.livestock.size()) {
            ImGui::SetNextWindowPos(ImVec2(canvas_p0.x + 15, canvas_p0.y + 15));
            ImGui::BeginChild("InspectorOverlay", ImVec2(350, 240), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
            auto &a = tank.livestock[inspected_animal_idx];
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Inspecting: %s", a->common_name.c_str());
            ImGui::Text("Scientific: %s", a->scientific_name.c_str());
            ImGui::Text("HP: %.1f / %.1f", a->health_hp, a->max_health_hp);
            ImGui::Text("Biomass: %.2f g", a->biomass_g);

            ImGui::SeparatorText("Tolerances");
            ImGui::Text("Temp: %.1f - %.1f °C", a->min_temp_c, a->max_temp_c);
            ImGui::Text("pH: %.1f - %.1f | GH: %.1f - %.1f", a->min_ph, a->max_ph, a->min_gh, a->max_gh);

            ImGui::Separator();
            if (!a->display_stresses.empty()) {
                for (const auto& w : a->display_stresses) ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "WARNUNG: %s", w.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "STATUS: Gesund & Aktiv");
            }
            ImGui::Separator();
            if (ImGui::Button("Close Inspector")) inspected_animal_idx = -1;
            ImGui::EndChild();

        } else if (inspected_plant_idx >= 0 && inspected_plant_idx < tank.flora.size()) {
            ImGui::SetNextWindowPos(ImVec2(canvas_p0.x + 15, canvas_p0.y + 15));
            ImGui::BeginChild("InspectorOverlay", ImVec2(350, 240), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
            auto &p = tank.flora[inspected_plant_idx];
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Inspecting: %s", p->common_name.c_str());
            ImGui::Text("HP: %.1f / %.1f", p->health_hp, p->max_health_hp);
            ImGui::Text("Height: %.1f cm (Max: %.1f cm)", p->current_height_cm, p->max_submersed_height_cm + p->max_emersed_height_cm);

            if (p->current_height_cm > p->max_submersed_height_cm) {
                ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Phase: Emers (Wächst über Wasser)");
            } else {
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Phase: Submers (Unter Wasser)");
            }

            ImGui::SeparatorText("Tolerances");
            ImGui::Text("Temp: %.1f - %.1f °C", p->min_temp_c, p->max_temp_c);
            ImGui::Text("pH: %.1f - %.1f | GH: %.1f - %.1f", p->min_ph, p->max_ph, p->min_gh, p->max_gh);

            ImGui::Separator();
            if (!p->display_stresses.empty()) {
                for (const auto& w : p->display_stresses) ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "%s", w.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "STATUS: Wächst optimal");
            }
            ImGui::Separator();
            if (ImGui::Button("Close Inspector")) inspected_plant_idx = -1;
            ImGui::EndChild();
        }
    }
};

#endif //AQUARIUMSIM_UIMANAGER_H