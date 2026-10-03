cmake_minimum_required(VERSION 3.20)
project(AquariumSim)

set(CMAKE_CXX_STANDARD 20)

# --- 1. Fetch GLFW ---
include(FetchContent)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# --- 2. Lokale ImGui Quelldateien sammeln ---
file(GLOB IMGUI_SOURCES
        external/imgui-1.92.9b/*.cpp
        external/imgui-1.92.9b/backends/imgui_impl_glfw.cpp
        external/imgui-1.92.9b/backends/imgui_impl_opengl3.cpp
)

# --- 3. Executable erstellen (inklusive neuer Ordner-Struktur) ---
add_executable(AquariumSim
        main.cpp
        ${IMGUI_SOURCES}
        Aquariums/TankModel.h
        Organisms/Animals/Animal.h
        Organisms/Plants/Plant.h
        Organisms/OrganismFactory.h
        Simulation/WaterParameters.h
        Simulation/ChemistryEngine.h
        Simulation/Aquarium.h
        Simulation/Items.h
        Frontend/UIManager.h
        Organisms/Animals/NeonTetra.h
        Organisms/Animals/NeocaridinaDavidi.h
        Organisms/Plants/Hornwort.h
        Organisms/Plants/AnubiasNana.h
        Organisms/Plants/HygrophilaPolyspermaRosanervig.h
        Organisms/Plants/RotalaOrangeJuice.h
        Organisms/Plants/Mossball.h
        Organisms/Plants/CryptocoryneWendtii.h
)

# Dem Compiler sagen, wo er die ImGui-Header findet
target_include_directories(AquariumSim PRIVATE
        external/imgui-1.92.9b
        external/imgui-1.92.9b/backends
)

# Bibliotheken verlinken
target_link_libraries(AquariumSim PRIVATE glfw)

if (UNIX AND NOT APPLE)
    target_link_libraries(AquariumSim PRIVATE m OpenGL)
elseif (WIN32)
    target_link_libraries(AquariumSim PRIVATE opengl32)
endif()