#ifndef SCENE_H
#define SCENE_H

#include "Flyweight.h"
#include "../Shader.h"
#include "zones/Environment.h"
#include "zones/BarrackGrid.h"
#include "zones/FenceSystem.h"
#include "zones/EntranceZone.h"
#include "zones/CrematoryZone.h"

class Scene {
public:
    void init();
    void render(Shader& shader);
    void updateTrainMovement(float forwardInput, float lateralInput, float deltaTime);
    void setBarrackDoorsOpen(bool open);
    void setInteriorLightsOn(bool on);

private:
    FlyweightFactory factory;
    Environment environment;
    BarrackGrid barrackGrid;
    FenceSystem fenceSystem;
    EntranceZone entranceZone;
    CrematoryZone crematoryZone;
};

#endif
