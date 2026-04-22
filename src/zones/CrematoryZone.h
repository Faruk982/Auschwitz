#ifndef CREMATORYZONE_H
#define CREMATORYZONE_H

#include "../Flyweight.h"
#include "../../Shader.h"

class CrematoryZone {
public:
    void init(FlyweightFactory& factory);
    void render(Shader& shader);
    void setDoorsOpen(bool open);
    void setInteriorLightsOn(bool on);
private:
    MeshFlyweight* darkBrickFlyweight;
    MeshFlyweight* chimneyFlyweight;
    MeshFlyweight* woodFlyweight;
    MeshFlyweight* cremRoofTriFlyweight;
    MeshFlyweight* crateFlyweight;
    MeshFlyweight* lampCanopyFlyweight;
    MeshFlyweight* lampHolderFlyweight;
    MeshFlyweight* bulbFlyweight;
    bool doorsOpen = false;
    bool interiorLightsOn = true;
};

#endif
