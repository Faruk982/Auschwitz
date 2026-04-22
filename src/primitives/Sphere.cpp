#include "Sphere.h"
#include <cmath>

Mesh Sphere::generate(int xSegments, int ySegments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const float PI = 3.14159265359f;

    for (int y = 0; y <= ySegments; ++y) {
        for (int x = 0; x <= xSegments; ++x) {
            float xSegment = (float)x / (float)xSegments;
            float ySegment = (float)y / (float)ySegments;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            Vertex vert;
            vert.Position = glm::vec3(xPos, yPos, zPos);
            vert.Normal = glm::vec3(xPos, yPos, zPos); // For a sphere centered at origin, normal = normalized position
            vert.TexCoords = glm::vec2(xSegment, ySegment);
            vertices.push_back(vert);
        }
    }

    bool oddRow = false;
    for (int y = 0; y < ySegments; ++y) {
        if (!oddRow) {
            for (int x = 0; x <= xSegments; ++x) {
                indices.push_back(y * (xSegments + 1) + x);
                indices.push_back((y + 1) * (xSegments + 1) + x);
            }
        } else {
            for (int x = xSegments; x >= 0; --x) {
                indices.push_back((y + 1) * (xSegments + 1) + x);
                indices.push_back(y * (xSegments + 1) + x);
            }
        }
        oddRow = !oddRow;
    }

    // Since the topology isn't simple triangles (it's triangle strips in Main.cpp), 
    // we should convert it to triangles for consistency in Mesh::Draw.
    // Let's rewrite index generation for GL_TRIANGLES:
    indices.clear();
    for (int y = 0; y < ySegments; ++y) {
        for (int x = 0; x < xSegments; ++x) {
            unsigned int top1 = y * (xSegments + 1) + x;
            unsigned int bot1 = (y + 1) * (xSegments + 1) + x;
            unsigned int top2 = y * (xSegments + 1) + x + 1;
            unsigned int bot2 = (y + 1) * (xSegments + 1) + x + 1;

            indices.push_back(top1);
            indices.push_back(bot1);
            indices.push_back(top2);

            indices.push_back(bot1);
            indices.push_back(bot2);
            indices.push_back(top2);
        }
    }

    return Mesh(vertices, indices);
}
