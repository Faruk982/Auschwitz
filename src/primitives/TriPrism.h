#ifndef TRIPRISM_H
#define TRIPRISM_H

#include "../Mesh.h"
#include <vector>

class TriPrism {
public:
    static Mesh generate() {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        auto pushFace = [&](const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
                            const glm::vec3& n) {
            unsigned int base = static_cast<unsigned int>(vertices.size());
            vertices.push_back({ p0, n, glm::vec2(0.0f, 0.0f) });
            vertices.push_back({ p1, n, glm::vec2(1.0f, 0.0f) });
            vertices.push_back({ p2, n, glm::vec2(1.0f, 1.0f) });
            vertices.push_back({ p3, n, glm::vec2(0.0f, 1.0f) });
            indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
            indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 3);
        };

        auto pushTri = [&](const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
                           const glm::vec3& n) {
            unsigned int base = static_cast<unsigned int>(vertices.size());
            vertices.push_back({ p0, n, glm::vec2(0.0f, 0.0f) });
            vertices.push_back({ p1, n, glm::vec2(1.0f, 0.0f) });
            vertices.push_back({ p2, n, glm::vec2(0.5f, 1.0f) });
            indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
        };

        const glm::vec3 a(-0.5f, -0.5f, -0.5f); // left-bottom-back
        const glm::vec3 b( 0.5f, -0.5f, -0.5f); // right-bottom-back
        const glm::vec3 c( 0.0f,  0.5f, -0.5f); // top-back

        const glm::vec3 d(-0.5f, -0.5f,  0.5f); // left-bottom-front
        const glm::vec3 e( 0.5f, -0.5f,  0.5f); // right-bottom-front
        const glm::vec3 f( 0.0f,  0.5f,  0.5f); // top-front

        // Triangular ends
        pushTri(a, b, c, glm::vec3(0.0f, 0.0f, -1.0f));
        pushTri(e, d, f, glm::vec3(0.0f, 0.0f, 1.0f));

        // Bottom face
        pushFace(d, e, b, a, glm::vec3(0.0f, -1.0f, 0.0f));

        // Left sloped face
        glm::vec3 nLeft = glm::normalize(glm::cross(c - a, d - a));
        pushFace(a, c, f, d, nLeft);

        // Right sloped face
        glm::vec3 nRight = glm::normalize(glm::cross(e - b, c - b));
        pushFace(b, e, f, c, nRight);

        return Mesh(vertices, indices);
    }
};

#endif
