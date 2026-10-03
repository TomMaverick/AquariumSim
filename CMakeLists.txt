cmake_minimum_required(VERSION 4.0)
project(AquariumSim)

set(CMAKE_CXX_STANDARD 20)

add_executable(AquariumSim main.cpp
        Simulation/WaterParameters.cpp
        Simulation/WaterParameters.h
        Simulation/ChemistryEngine.cpp
        Simulation/ChemistryEngine.h
        Simulation/Aquarium.cpp
        Simulation/Aquarium.h
        Simulation/Organisms.cpp
        Simulation/Organisms.h)
