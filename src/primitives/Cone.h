#ifndef CONE_H
#define CONE_H

#include "../Mesh.h"
#include <vector>
#include <cmath>

class Cone {
public:
    static Mesh generate(int segments = 24) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        const float PI = 3.14159265359f;
        const glm::vec3 tip(0.0f, 0.5f, 0.0f);
        const float radius = 0.5f;
        const float baseY = -0.5f;

        for (int i = 0; i <= segments; ++i) {
            float t = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
            float x = radius * std::cos(t);
            float z = radius * std::sin(t);
            glm::vec3 p(x, baseY, z);

            glm::vec3 tangent(-std::sin(t), 0.0f, std::cos(t));
            glm::vec3 toTip = glm::normalize(tip - p);
            glm::vec3 normal = glm::normalize(glm::cross(tangent, toTip));

            vertices.push_back({ tip, normal, glm::vec2(static_cast<float>(i) / segments, 1.0f) });
            vertices.push_back({ p, normal, glm::vec2(static_cast<float>(i) / segments, 0.0f) });
        }

        for (int i = 0; i < segments; ++i) {
            unsigned int tip1 = i * 2;
            unsigned int base1 = i * 2 + 1;
            unsigned int tip2 = (i + 1) * 2;
            unsigned int base2 = (i + 1) * 2 + 1;

            indices.push_back(tip1);
            indices.push_back(base1);
            indices.push_back(base2);

            indices.push_back(tip1);
            indices.push_back(base2);
            indices.push_back(tip2);
        }

        unsigned int centerIndex = static_cast<unsigned int>(vertices.size());
        vertices.push_back({ glm::vec3(0.0f, baseY, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f, 0.5f) });

        for (int i = 0; i < segments; ++i) {
            float t = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
            float x = radius * std::cos(t);
            float z = radius * std::sin(t);
            vertices.push_back({ glm::vec3(x, baseY, z), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f + 0.5f * std::cos(t), 0.5f + 0.5f * std::sin(t)) });
        }

        for (int i = 0; i < segments; ++i) {
            indices.push_back(centerIndex);
            indices.push_back(centerIndex + 1 + ((i + 1) % segments));
            indices.push_back(centerIndex + 1 + i);
        }

        return Mesh(vertices, indices);
    }
};

#endif
