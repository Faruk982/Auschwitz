#ifndef SPHERE_H
#define SPHERE_H

#include "../Mesh.h"

class Sphere {
public:
    static Mesh generate(int xSegments = 64, int ySegments = 64);
};

#endif
