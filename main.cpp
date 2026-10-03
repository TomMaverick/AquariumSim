#include <iostream>
#include <chrono>
#include <thread>

// ImGui & GLFW Headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// Your Simulation Headers
#include "Simulation/WaterParameters.h"
#include "Simulation/Aquarium.h"
#include "Frontend/UIManager.h"

// Error callback for GLFW
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
    initialWater.nh4 = 1.0;
    initialWater.no3 = 10.0;

    // --- NEU: Becken über das TankModel initialisieren ---
    TankModel myTank = TankModel::dennerleNanoCube30();
    Aquarium nanoCube(myTank, initialWater);

    nanoCube.addSpongeFilter();
    nanoCube.addPlant(OrganismFactory::createHornwortStem());
    nanoCube.addPlant(OrganismFactory::createAnubias());
    for(int i = 0; i < 26; i++) {
        nanoCube.addAnimal(OrganismFactory::createNeocaridina());
    }

    UIManager uiManager;
    uiManager.initHistory(nanoCube);

    int tickCounter = 0;
    auto lastTime = std::chrono::steady_clock::now();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastTime;

        if (elapsed.count() >= 0.1) {
            lastTime = currentTime;
            if (!uiManager.isPaused()) {
                tickCounter++;
                nanoCube.update(1.0);
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