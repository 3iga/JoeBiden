#pragma once
#include "MathSupport.hpp"
#include <array>

constexpr int SpeciesCount=8;
enum class Behaviour { Hover, Cruise, Graze, Shoal, Heavy };
struct Species {
    const char* name;
    const char* scientific;
    const char* description;
    const char* fact;
    const char* source;
    const char* photoFile;
    float maximumLengthCm;   // Sourced species maximum, NOT a measured specimen.
    float modelMassKg;       // Illustrative simulation parameter, NOT a biological record.
    float speed;             // Scene units / second, chosen for the graphics demo.
    Vec3 bodyShape;
    Vec4 color;
    Behaviour behaviour;
};
extern const std::array<Species,SpeciesCount> SpeciesList;
const char* behaviourName(Behaviour b);
