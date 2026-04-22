#ifndef ENTRANCEZONE_H
#define ENTRANCEZONE_H

#include "../Flyweight.h"
#include "../../Shader.h"
#include "../Mesh.h"

class EntranceZone {
public:
    void init(FlyweightFactory& factory);
    void render(Shader& shader);
    void updateTrainMovement(float forwardInput, float lateralInput, float deltaTime);

private:
    // Gate structure
    MeshFlyweight* gatehouseFlyweight;   // brick_dark walls
    MeshFlyweight* roofFlyweight;        // roof_tile pitched roofs
    MeshFlyweight* gateRedBrick;         // brick_red for extended wings
    MeshFlyweight* windowFlyweight;      // dark inset cubes for windows

    // Train tracks
    MeshFlyweight* railFlyweight;        // metal_iron rails
    MeshFlyweight* sleeperFlyweight;     // wood_plank sleepers

    // Train car (cattle wagon)
    MeshFlyweight* trainBodyFlyweight;   // wood_plank wagon body
    MeshFlyweight* trainRoofFlyweight;   // brick_red barrel roof (cylinder)
    MeshFlyweight* trainWheelFlyweight;  // metal_iron undercarriage
    MeshFlyweight* trainFrontFlyweight;  // container2 texture for front section

    // Left entrance road
    MeshFlyweight* roadFlyweight;        // dirt_road for secondary path

    // Roof prism
    MeshFlyweight* roofPrismFlyweight;   // TriPrism with roof_tile

    // Train movement state
    float trainOffsetX = 0.0f;
    float trainOffsetZ = 0.0f;

    // Helper methods
    void renderGatehouse(Shader& shader);
    void renderTrainTracks(Shader& shader);
    void renderTrainCars(Shader& shader);
    void renderLeftEntrance(Shader& shader);
};

#endif
