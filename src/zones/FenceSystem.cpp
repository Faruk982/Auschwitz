#include "FenceSystem.h"
#include "../primitives/Cube.h"
#include "../primitives/Cylinder.h"
#include "../Texture.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace {
float hash01(float n) {
    float s = std::sin(n * 12.9898f) * 43758.5453f;
    return s - std::floor(s);
}

glm::vec3 rotateAround(const glm::vec3& v, const glm::vec3& axis, float angleRad) {
    glm::mat4 r = glm::rotate(glm::mat4(1.0f), angleRad, axis);
    return glm::vec3(r * glm::vec4(v, 0.0f));
}
}

void FenceSystem::init(FlyweightFactory& factory) {
    // Fence posts (concrete cylinders)
    if (!factory.get("fence_post")) {
        unsigned int tex = loadTexture("textures/concrete.png", true);
        factory.add("fence_post", Cylinder::generate(12), tex);
    }
    postFlyweight = factory.get("fence_post");

    // Side gate masonry
    if (!factory.get("side_gate_masonry")) {
        unsigned int tex = loadTexture("textures/brick_dark.png", true);
        factory.add("side_gate_masonry", Cube::generate(), tex);
    }
    sideGateFlyweight = factory.get("side_gate_masonry");

    // Tower base (wooden cylinder)
    if (!factory.get("tower_base")) {
        unsigned int tex = loadTexture("textures/wood_plank.png", true);
        factory.add("tower_base", Cylinder::generate(8), tex);
    }
    towerBaseFlyweight = factory.get("tower_base");

    // Tower cabin (wooden box)
    if (!factory.get("tower_cabin")) {
        unsigned int tex = loadTexture("textures/wood_plank.png", true);
        factory.add("tower_cabin", Cube::generate(), tex);
    }
    towerCabinFlyweight = factory.get("tower_cabin");

    // Tower roof 
    if (!factory.get("tower_roof")) {
        unsigned int tex = loadTexture("textures/roof_tile.png", true);
        factory.add("tower_roof", Cube::generate(), tex);
    }
    towerRoofFlyweight = factory.get("tower_roof");

    // Wire (thin cylinder between posts)
    if (!factory.get("wire")) {
        unsigned int tex = loadTexture("textures/metal_iron.png", true);
        factory.add("wire", Cylinder::generate(6), tex);
    }
    wireFlyweight = factory.get("wire");

    // Tree branches
    if (!factory.get("tree_branch")) {
        unsigned int tex = loadTexture("textures/bark.png", true);
        factory.add("tree_branch", Cylinder::generate(8), tex);
    }
    treeBranchFlyweight = factory.get("tree_branch");

    postPositions.clear();
    towerPositions.clear();
    treePositions.clear();

    // Generate fence posts around perimeter
    // Perimeter: X: [-280, +175], Z: [-90, +100]
    float minX = -280.0f, maxX = 175.0f;
    float minZ = -90.0f, maxZ = 100.0f;
    const float gateHalfGap = 12.0f;
    const float sideGateHalfGap = 4.0f;
    const float sideGateZ[] = { -30.0f, 30.0f };
    float postSpacing = 3.0f;

    // North fence (Z = maxZ)
    for (float x = minX; x <= maxX; x += postSpacing)
        postPositions.push_back(glm::vec3(x, 0.0f, maxZ));
    // South fence (Z = minZ)
    for (float x = minX; x <= maxX; x += postSpacing)
        postPositions.push_back(glm::vec3(x, 0.0f, minZ));
    // East fence (X = maxX) — skip main and side gate areas
    for (float z = minZ; z <= maxZ; z += postSpacing) {
        if (z > -gateHalfGap && z < gateHalfGap) continue;

        bool inSideGate = false;
        for (float gz : sideGateZ) {
            if (z > gz - sideGateHalfGap && z < gz + sideGateHalfGap) {
                inSideGate = true;
                break;
            }
        }
        if (inSideGate) continue;

        postPositions.push_back(glm::vec3(maxX, 0.0f, z));
    }
    // West fence (X = minX)
    for (float z = minZ; z <= maxZ; z += postSpacing)
        postPositions.push_back(glm::vec3(minX, 0.0f, z));

    // Border trees (inside fence)
    const float treeSpacing = 22.0f;
    for (float x = minX + 10.0f; x <= maxX - 14.0f; x += treeSpacing) {
        treePositions.push_back(glm::vec3(x, 0.0f, maxZ - 6.0f));
        treePositions.push_back(glm::vec3(x, 0.0f, minZ + 6.0f));
    }

    for (float z = minZ + 12.0f; z <= maxZ - 12.0f; z += treeSpacing) {
        treePositions.push_back(glm::vec3(minX + 6.0f, 0.0f, z));

        if (z > -gateHalfGap - 9.0f && z < gateHalfGap + 9.0f) continue;

        bool nearSideGate = false;
        for (float gz : sideGateZ) {
            if (z > gz - sideGateHalfGap - 8.0f && z < gz + sideGateHalfGap + 8.0f) {
                nearSideGate = true;
                break;
            }
        }
        if (!nearSideGate)
            treePositions.push_back(glm::vec3(maxX - 6.0f, 0.0f, z));
    }

    // Guard towers at corners and intervals
    towerPositions = {
        // Corners
        {minX, 0.0f, maxZ}, {maxX, 0.0f, maxZ},
        {minX, 0.0f, minZ}, {maxX, 0.0f, minZ},
        // North side
        {-100.0f, 0.0f, maxZ}, {-40.0f, 0.0f, maxZ}, {20.0f, 0.0f, maxZ},
        // South side
        {-100.0f, 0.0f, minZ}, {-40.0f, 0.0f, minZ}, {20.0f, 0.0f, minZ},
        // Gate towers
        {maxX, 0.0f, -14.0f}, {maxX, 0.0f, 14.0f}
    };
}

void FenceSystem::renderTower(Shader& shader, const glm::vec3& pos) {
    // Tower = 4 leg posts + cabin + roof
    float legHeight = 5.0f;
    float cabinSize = 3.0f;
    float legSpread = 1.2f;

    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 0);
    shader.setFloat("textureTiling", 2.0f);
    shader.setFloat("shininess", 8.0f);

    // 4 legs (cylinder posts)
    for (int dx = -1; dx <= 1; dx += 2) {
        for (int dz = -1; dz <= 1; dz += 2) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pos + glm::vec3(dx * legSpread, legHeight / 2.0f, dz * legSpread));
            model = glm::scale(model, glm::vec3(0.3f, legHeight, 0.3f));
            shader.setMat4("model", model);
            towerBaseFlyweight->draw(shader);
        }
    }

    // Cabin (box)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos + glm::vec3(0.0f, legHeight + cabinSize / 2.0f, 0.0f));
    model = glm::scale(model, glm::vec3(cabinSize, cabinSize, cabinSize));
    shader.setMat4("model", model);
    towerCabinFlyweight->draw(shader);

    // Roof (slightly wider flattened box)
    model = glm::mat4(1.0f);
    model = glm::translate(model, pos + glm::vec3(0.0f, legHeight + cabinSize + 0.5f, 0.0f));
    model = glm::scale(model, glm::vec3(cabinSize + 1.0f, 1.0f, cabinSize + 1.0f));
    shader.setMat4("model", model);
    towerRoofFlyweight->draw(shader);
}

void FenceSystem::renderTreeBranch(Shader& shader, const glm::vec3& start, const glm::vec3& dir, float length, float radius, int depth, int maxDepth, float seed) {
    glm::vec3 nDir = glm::normalize(dir);
    glm::vec3 end = start + nDir * length;

    bool leafy = depth >= maxDepth - 2;
    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 1);
    shader.setFloat("textureBlendFactor", leafy ? 0.95f : 0.7f);
    shader.setFloat("textureTiling", 1.0f);
    shader.setFloat("shininess", leafy ? 2.0f : 4.0f);
    if (leafy) shader.setVec3("objectColor", 0.07f, 0.47f, 0.09f);
    else shader.setVec3("objectColor", 0.42f, 0.21f, 0.08f);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, start + nDir * (length * 0.5f));

    const glm::vec3 up(0.0f, 1.0f, 0.0f);
    float c = glm::clamp(glm::dot(up, nDir), -1.0f, 1.0f);
    float angle = std::acos(c);
    glm::vec3 axis = glm::cross(up, nDir);
    float axisLen = glm::length(axis);
    if (axisLen > 1e-4f) {
        model = glm::rotate(model, angle, axis / axisLen);
    } else if (c < 0.0f) {
        model = glm::rotate(model, glm::pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    model = glm::scale(model, glm::vec3(radius, length, radius));
    shader.setMat4("model", model);
    treeBranchFlyweight->draw(shader);

    if (depth >= maxDepth) return;

    glm::vec3 side = glm::cross(nDir, glm::vec3(0.0f, 1.0f, 0.0f));
    if (glm::length(side) < 1e-4f) side = glm::vec3(1.0f, 0.0f, 0.0f);
    side = glm::normalize(side);

    const int childCount = depth >= maxDepth - 2 ? 3 : 2;
    float nextLength = length * (depth >= maxDepth - 2 ? 0.66f : 0.72f);
    float nextRadius = radius * 0.72f;

    for (int i = 0; i < childCount; ++i) {
        float a = hash01(seed * 17.0f + depth * 11.0f + i * 7.0f);
        float b = hash01(seed * 31.0f + depth * 13.0f + i * 19.0f);

        float bendDeg = 10.0f + a * 40.0f;
        float azimuthDeg = (360.0f / childCount) * i + (b - 0.5f) * 40.0f;

        glm::vec3 childDir = rotateAround(nDir, side, glm::radians(bendDeg));
        childDir = rotateAround(childDir, nDir, glm::radians(azimuthDeg));

        renderTreeBranch(shader, end, childDir, nextLength, nextRadius, depth + 1, maxDepth, seed + i * 3.1f + a * 5.7f);
    }
}

void FenceSystem::renderTree(Shader& shader, const glm::vec3& pos, float seed) {
    renderTreeBranch(shader, pos, glm::vec3(0.0f, 1.0f, 0.0f), 4.8f, 0.22f, 0, 4, seed);
}

void FenceSystem::render(Shader& shader) {
    // === FENCE POSTS ===
    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 0);
    shader.setFloat("textureTiling", 1.0f);
    shader.setFloat("shininess", 8.0f);

    float postHeight = 2.5f;
    for (const auto& pos : postPositions) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos + glm::vec3(0.0f, postHeight / 2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.15f, postHeight, 0.15f));
        shader.setMat4("model", model);
        postFlyweight->draw(shader);
    }

    // === WIRE STRANDS between posts ===
    shader.setFloat("textureTiling", 10.0f);
    float wireHeights[] = { 0.8f, 1.5f, 2.2f };
    float minX = -280.0f, maxX = 175.0f;
    float minZ = -90.0f, maxZ = 100.0f;
    const float gateHalfGap = 12.0f;
    const float sideGateHalfGap = 4.0f;

    auto drawEastWireSegment = [&](float h, float z0, float z1) {
        if (z1 <= z0) return;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(maxX, h, (z0 + z1) * 0.5f));
        model = glm::scale(model, glm::vec3(0.03f, 0.03f, z1 - z0));
        shader.setMat4("model", model);
        wireFlyweight->draw(shader);
    };

    for (float h : wireHeights) {
        // North wire
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3((minX + maxX) * 0.5f, h, maxZ));
        model = glm::scale(model, glm::vec3(maxX - minX, 0.03f, 0.03f));
        shader.setMat4("model", model);
        wireFlyweight->draw(shader);

        // South wire
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3((minX + maxX) * 0.5f, h, minZ));
        model = glm::scale(model, glm::vec3(maxX - minX, 0.03f, 0.03f));
        shader.setMat4("model", model);
        wireFlyweight->draw(shader);

        // West wire
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(minX, h, (minZ + maxZ) * 0.5f));
        model = glm::scale(model, glm::vec3(0.03f, 0.03f, maxZ - minZ));
        shader.setMat4("model", model);
        wireFlyweight->draw(shader);

        // East wire with three gate gaps (left side, main, right side)
        drawEastWireSegment(h, minZ, -30.0f - sideGateHalfGap);
        drawEastWireSegment(h, -30.0f + sideGateHalfGap, -gateHalfGap);
        drawEastWireSegment(h, gateHalfGap, 30.0f - sideGateHalfGap);
        drawEastWireSegment(h, 30.0f + sideGateHalfGap, maxZ);
    }

    // === GUARD TOWERS ===
    for (const auto& pos : towerPositions) {
        renderTower(shader, pos);
    }

    // === SIDE GATES (left and right) ===
    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 0);
    shader.setFloat("textureTiling", 2.0f);
    shader.setFloat("shininess", 4.0f);

    const float sideGateZ[] = { -30.0f, 30.0f };
    const float gateHeight = 4.8f;
    const float pillarThickness = 0.75f;
    const float clearWidth = 8.0f;

    for (float z : sideGateZ) {
        for (int s = -1; s <= 1; s += 2) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(maxX, gateHeight * 0.5f, z + s * (clearWidth * 0.5f + pillarThickness * 0.5f)));
            model = glm::scale(model, glm::vec3(1.4f, gateHeight, pillarThickness));
            shader.setMat4("model", model);
            sideGateFlyweight->draw(shader);
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(maxX, gateHeight + 0.35f, z));
        model = glm::scale(model, glm::vec3(1.4f, 0.7f, clearWidth + 2.0f * pillarThickness));
        shader.setMat4("model", model);
        sideGateFlyweight->draw(shader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(maxX, gateHeight + 0.85f, z));
        model = glm::scale(model, glm::vec3(1.6f, 0.25f, clearWidth + 2.6f * pillarThickness));
        shader.setMat4("model", model);
        towerRoofFlyweight->draw(shader);
    }

    // === BORDER TREES ===
    float seed = 3.17f;
    for (const auto& pos : treePositions) {
        renderTree(shader, pos, seed);
        seed += 1.618f;
    }
}
