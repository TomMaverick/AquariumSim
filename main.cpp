#include <iostream>
#include <chrono>
#include <thread>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "Simulation/WaterParameters.h"
#include "Simulation/Aquarium.h"
#include "Frontend/UIManager.h"

static void glfw_error_callback(int error, const char *description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int main() {
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Aquarium Simulation Engine", nullptr, nullptr);
    if (window == nullptr) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    WaterParameters initialWater = WaterParameters::createAverageTapWater(22.0);
    initialWater.nh4 = 0.0;
    initialWater.no3 = 5.0;

    TankModel myTank = TankModel::dennerleNanoCube30();
    Aquarium nanoCube(myTank, initialWater);
    nanoCube.addSpongeFilter();

    // Tiere und Pflanzen wurden entfernt - das Becken startet leer!

    UIManager uiManager;
    uiManager.initHistory(nanoCube);

    int tickCounter = 0; // Entspricht Stunden
    double hour_accumulator = 0.0;
    auto lastTime = std::chrono::steady_clock::now();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastTime;
        lastTime = currentTime;

        if (!uiManager.isPaused() && elapsed.count() < 0.5) { // Schutz vor großen Zeitsprüngen beim Minimieren
            double real_delta_seconds = elapsed.count();
            double simulation_hours_per_real_second = 1.0;

            switch (uiManager.speed_mode) {
                case 0: simulation_hours_per_real_second = 1.0 / 3600.0; break; // Echtzeit (1 Sim-Sekunde / Sekunde)
                case 1: simulation_hours_per_real_second = 1.0 / 60.0;   break; // 1 min / s
                case 2: simulation_hours_per_real_second = 10.0 / 60.0;  break; // 10 min / s
                case 3: simulation_hours_per_real_second = 1.0;          break; // 1 h / s
                case 4: simulation_hours_per_real_second = 12.0;         break; // 12 h / s
                case 5: simulation_hours_per_real_second = 24.0;         break; // 24 h / s (1 Tag / s)
            }

            double sim_hours_passed = simulation_hours_per_real_second * real_delta_seconds;

            if (sim_hours_passed > 0.0) {
                hour_accumulator += sim_hours_passed;

                // Sobald eine volle Stunde vergangen ist, erhöhen wir den Tick-Counter
                while (hour_accumulator >= 1.0) {
                    tickCounter++;
                    hour_accumulator -= 1.0;
                }

                nanoCube.update(sim_hours_passed, tickCounter);
                uiManager.updateHistory(nanoCube);
            }
        }

        uiManager.render(nanoCube, tickCounter);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.05f, 0.05f, 0.05f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}