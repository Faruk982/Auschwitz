#ifndef BARRACKGRID_H
#define BARRACKGRID_H

#include "../Flyweight.h"
#include "../../Shader.h"
#include <vector>
#include <glm/glm.hpp>

struct BarrackInstance {
    glm::vec3 position;
    bool isDark; // Block 11 variant
};

class BarrackGrid {
public:
    void init(FlyweightFactory& factory);
    void render(Shader& shader);
    void setDoorsOpen(bool open);
    void setInteriorLightsOn(bool on);
private:
    MeshFlyweight* wallFlyweight;
    MeshFlyweight* wallDarkFlyweight;
    MeshFlyweight* roofTriFlyweight;
    MeshFlyweight* doorFlyweight;
    MeshFlyweight* gableFlyweight;
    MeshFlyweight* gableDarkFlyweight;
    MeshFlyweight* bedWoodFlyweight;
    MeshFlyweight* bedMattressFlyweight;
    MeshFlyweight* lampCanopyFlyweight;
    MeshFlyweight* lampHolderFlyweight;
    MeshFlyweight* bulbFlyweight;
    std::vector<BarrackInstance> instances;
    bool doorsOpen = false;
    bool interiorLightsOn = true;
    
    void renderBarrack(Shader& shader, const BarrackInstance& inst);
};

#endif
