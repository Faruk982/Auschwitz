#include "Plane.h"

Mesh Plane::generate() {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float v[] = {
        // positions          // normals         // texcoords
         0.5f,  0.0f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.0f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
        -0.5f,  0.0f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         0.5f,  0.0f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
    };

    for(int i = 0; i < 4; ++i) {
        Vertex vert;
        vert.Position  = glm::vec3(v[i*8], v[i*8+1], v[i*8+2]);
        vert.Normal    = glm::vec3(v[i*8+3], v[i*8+4], v[i*8+5]);
        vert.TexCoords = glm::vec2(v[i*8+6], v[i*8+7]);
        vertices.push_back(vert);
    }

    indices = {
        0, 1, 2, 
        0, 2, 3
    };

    return Mesh(vertices, indices);
}
