#include "Cylinder.h"
#include <cmath>

Mesh Cylinder::generate(int segments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const float PI = 3.14159265359f;
    float radius = 0.5f;
    float halfHeight = 0.5f;

    // Side vertices
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * PI * float(i) / float(segments);
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);
        float u = float(i) / float(segments);

        // Top vertex
        Vertex topVert;
        topVert.Position = glm::vec3(x, halfHeight, z);
        topVert.Normal = glm::vec3(x, 0.0f, z);
        topVert.TexCoords = glm::vec2(u, 1.0f);
        vertices.push_back(topVert);

        // Bottom vertex
        Vertex botVert;
        botVert.Position = glm::vec3(x, -halfHeight, z);
        botVert.Normal = glm::vec3(x, 0.0f, z);
        botVert.TexCoords = glm::vec2(u, 0.0f);
        vertices.push_back(botVert);
    }

    // Side indices
    for (int i = 0; i < segments; ++i) {
        unsigned int top1 = i * 2;
        unsigned int bot1 = i * 2 + 1;
        unsigned int top2 = (i + 1) * 2;
        unsigned int bot2 = (i + 1) * 2 + 1;

        indices.push_back(top1);
        indices.push_back(bot1);
        indices.push_back(top2);

        indices.push_back(bot1);
        indices.push_back(bot2);
        indices.push_back(top2);
    }

    // Top and Bottom caps can be added similarly, but for most use cases (trees, posts) sides are enough,
    // or we add them for completeness. Let's add them.
    int topCenterIdx = vertices.size();
    Vertex topCenter;
    topCenter.Position = glm::vec3(0, halfHeight, 0);
    topCenter.Normal = glm::vec3(0, 1, 0);
    topCenter.TexCoords = glm::vec2(0.5f, 0.5f);
    vertices.push_back(topCenter);

    for (int i = 0; i < segments; ++i) {
        float theta = 2.0f * PI * float(i) / float(segments);
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);
        float nx = std::cos(theta);
        float nz = std::sin(theta);

        Vertex capVert;
        capVert.Position = glm::vec3(x, halfHeight, z);
        capVert.Normal = glm::vec3(0, 1, 0);
        capVert.TexCoords = glm::vec2(0.5f + 0.5f * nx, 0.5f + 0.5f * nz);
        vertices.push_back(capVert);
    }
    for (int i = 0; i < segments; ++i) {
        indices.push_back(topCenterIdx);
        indices.push_back(topCenterIdx + 1 + i);
        indices.push_back(topCenterIdx + 1 + ((i + 1) % segments));
    }

    int botCenterIdx = vertices.size();
    Vertex botCenter;
    botCenter.Position = glm::vec3(0, -halfHeight, 0);
    botCenter.Normal = glm::vec3(0, -1, 0);
    botCenter.TexCoords = glm::vec2(0.5f, 0.5f);
    vertices.push_back(botCenter);

    for (int i = 0; i < segments; ++i) {
        float theta = 2.0f * PI * float(i) / float(segments);
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);
        float nx = std::cos(theta);
        float nz = std::sin(theta);

        Vertex capVert;
        capVert.Position = glm::vec3(x, -halfHeight, z);
        capVert.Normal = glm::vec3(0, -1, 0);
        capVert.TexCoords = glm::vec2(0.5f - 0.5f * nx, 0.5f + 0.5f * nz);
        vertices.push_back(capVert);
    }
    for (int i = 0; i < segments; ++i) {
        indices.push_back(botCenterIdx);
        indices.push_back(botCenterIdx + 1 + ((i + 1) % segments));
        indices.push_back(botCenterIdx + 1 + i);
    }

    return Mesh(vertices, indices);
}
