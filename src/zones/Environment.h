#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../Flyweight.h"
#include "../../Shader.h"

class Environment {
public:
    void init(FlyweightFactory& factory);
    void render(Shader& shader);
private:
    MeshFlyweight* groundFlyweight;
    MeshFlyweight* roadFlyweight;
    MeshFlyweight* railFlyweight;
    MeshFlyweight* sleeperFlyweight;
    MeshFlyweight* lampSphereFlyweight;
};

#endif
