#ifndef CYLINDER_H
#define CYLINDER_H

#include "../Mesh.h"

class Cylinder {
public:
    static Mesh generate(int segments = 36);
};

#endif
