#include "BarrackGrid.h"
#include "../primitives/Cube.h"
#include "../primitives/TriPrism.h"
#include "../primitives/Cylinder.h"
#include "../primitives/Cone.h"
#include "../primitives/Sphere.h"
#include "../Texture.h"
#include <glm/gtc/matrix_transform.hpp>

void BarrackGrid::init(FlyweightFactory& factory) {
    // Walls — red brick
    if (!factory.get("barrack_wall")) {
        unsigned int tex = loadTexture("textures/brick_red.jpg", true);
        factory.add("barrack_wall", Cube::generate(), tex);
    }
    wallFlyweight = factory.get("barrack_wall");

    // Walls — dark brick (Block 11)
    if (!factory.get("barrack_dark")) {
        unsigned int tex = loadTexture("textures/brick_dark.png", true);
        factory.add("barrack_dark", Cube::generate(), tex);
    }
    wallDarkFlyweight = factory.get("barrack_dark");

    // Triangular roof body
    if (!factory.get("barrack_roof_tri")) {
        unsigned int tex = loadTexture("textures/roof_tile.png", true);
        factory.add("barrack_roof_tri", TriPrism::generate(), tex);
    }
    roofTriFlyweight = factory.get("barrack_roof_tri");

    // Doors
    if (!factory.get("barrack_door")) {
        unsigned int tex = loadTexture("textures/wood_plank.png", true);
        factory.add("barrack_door", Cube::generate(), tex);
    }
    doorFlyweight = factory.get("barrack_door");

    // Gable end fills (triangular)
    if (!factory.get("barrack_gable")) {
        unsigned int tex = loadTexture("textures/brick_red.jpg", true);
        factory.add("barrack_gable", TriPrism::generate(), tex);
    }
    gableFlyweight = factory.get("barrack_gable");

    if (!factory.get("barrack_gable_dark")) {
        unsigned int tex = loadTexture("textures/brick_dark.png", true);
        factory.add("barrack_gable_dark", TriPrism::generate(), tex);
    }
    gableDarkFlyweight = factory.get("barrack_gable_dark");

    if (!factory.get("barrack_bed_wood")) {
        unsigned int tex = loadTexture("textures/wood_plank.png", true);
        factory.add("barrack_bed_wood", Cube::generate(), tex);
    }
    bedWoodFlyweight = factory.get("barrack_bed_wood");

    if (!factory.get("barrack_mattress")) {
        unsigned int tex = loadTexture("textures/istockphoto-1155534565-1024x1024.jpg", true);
        factory.add("barrack_mattress", Cube::generate(), tex);
    }
    bedMattressFlyweight = factory.get("barrack_mattress");

    if (!factory.get("barrack_lamp_canopy")) {
        unsigned int tex = loadTexture("textures/metal_grey.png", true);
        factory.add("barrack_lamp_canopy", Cylinder::generate(18), tex);
    }
    lampCanopyFlyweight = factory.get("barrack_lamp_canopy");

    if (!factory.get("barrack_lamp_holder")) {
        unsigned int tex = loadTexture("textures/metal_grey.png", true);
        factory.add("barrack_lamp_holder", Cone::generate(20), tex);
    }
    lampHolderFlyweight = factory.get("barrack_lamp_holder");

    if (!factory.get("barrack_bulb")) {
        unsigned int tex = loadTexture("textures/metal_grey.png", true);
        factory.add("barrack_bulb", Sphere::generate(16, 16), tex);
    }
    bulbFlyweight = factory.get("barrack_bulb");

    instances.clear();

    // Existing barracks near entrance gate (3x3 per side)
    const int rows = 3;
    const int lanes = 3;
    const float spacingX = 30.0f;
    const float laneOffsetZ[] = { 20.0f, 46.0f, 72.0f };
    const float rightSideExtraZ = 12.0f;

    const float existingStartX = 82.0f;
    for (int row = 0; row < rows; ++row) {
        float x = existingStartX - row * spacingX;

        for (int side = -1; side <= 1; side += 2) {
            for (int lane = 0; lane < lanes; ++lane) {
                BarrackInstance inst;
                float z = side * laneOffsetZ[lane] + (side > 0 ? rightSideExtraZ : 0.0f);
                inst.position = glm::vec3(x, 0.0f, z);
                inst.isDark = false;
                instances.push_back(inst);
            }
        }
    }

    // Separate Block 11 square behind the existing barracks (3x3 per side)
    // A road gap is left between these two groups.
    const float block11StartX = -48.0f;
    for (int row = 0; row < rows; ++row) {
        float x = block11StartX - row * spacingX;

        for (int side = -1; side <= 1; side += 2) {
            for (int lane = 0; lane < lanes; ++lane) {
                BarrackInstance inst;
                float z = side * laneOffsetZ[lane] + (side > 0 ? rightSideExtraZ : 0.0f);
                inst.position = glm::vec3(x, 0.0f, z);
                inst.isDark = true;
                instances.push_back(inst);
            }
        }
    }
}

void BarrackGrid::renderBarrack(Shader& shader, const BarrackInstance& inst) {
    float width = 13.0f;   // X
    float height = 8.0f;   // Y (two storeys)
    float length = 30.0f;  // Z
    const float yaw = glm::radians(90.0f);

    MeshFlyweight* wallMesh = inst.isDark ? wallDarkFlyweight : wallFlyweight;
    MeshFlyweight* gableMesh = inst.isDark ? gableDarkFlyweight : gableFlyweight;

    shader.setFloat("emissionStrength", 0.0f);

    // === WALLS (door opening only; top remains closed) ===
    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 0); // texture only
    shader.setFloat("textureTiling", 4.0f);
    shader.setFloat("shininess", 4.0f);
    shader.setVec3("objectColor", inst.isDark ? glm::vec3(0.3f, 0.22f, 0.18f) : glm::vec3(0.65f, 0.3f, 0.2f));

    const float wallThickness = 0.28f;
    const float doorHeight = 3.8f;
    const float doorWidth = 2.0f;
    const float doorDepth = 0.22f;

    auto drawWallSegment = [&](const glm::vec3& localOffset, const glm::vec3& localScale, float tiling = 4.0f) {
        shader.setFloat("textureTiling", tiling);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, inst.position + glm::vec3(0.0f, height / 2.0f, 0.0f));
        model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, localOffset);
        model = glm::scale(model, localScale);
        shader.setMat4("model", model);
        wallMesh->draw(shader);
    };

    // Long side walls
    drawWallSegment(glm::vec3(0.0f, 0.0f, length * 0.5f - wallThickness * 0.5f), glm::vec3(width, height, wallThickness), 4.0f);
    drawWallSegment(glm::vec3(0.0f, 0.0f, -length * 0.5f + wallThickness * 0.5f), glm::vec3(width, height, wallThickness), 4.0f);

    // End walls split around centered doorway opening
    const float sideWallWidth = (length - doorWidth) * 0.5f;
    const float topWallHeight = height - doorHeight;
    const float sideSegmentOffsetZ = (doorWidth + sideWallWidth) * 0.5f;
    const float topSegmentTiling = 4.0f * (topWallHeight / height);

    for (int side = -1; side <= 1; side += 2) {
        const float endX = side * (width * 0.5f - wallThickness * 0.5f);
        drawWallSegment(glm::vec3(endX, 0.0f, sideSegmentOffsetZ), glm::vec3(wallThickness, height, sideWallWidth), 4.0f);
        drawWallSegment(glm::vec3(endX, 0.0f, -sideSegmentOffsetZ), glm::vec3(wallThickness, height, sideWallWidth), 4.0f);
        drawWallSegment(glm::vec3(endX, doorHeight * 0.5f, 0.0f), glm::vec3(wallThickness, topWallHeight, doorWidth), topSegmentTiling);
    }

    glm::mat4 model = glm::mat4(1.0f);

    // Flat roof material override (wood plank) on the existing top face
    shader.setFloat("textureTiling", 3.0f);
    shader.setFloat("shininess", 2.0f);
    shader.setVec3("objectColor", glm::vec3(0.45f, 0.34f, 0.22f));

    model = glm::mat4(1.0f);
    model = glm::translate(model, inst.position + glm::vec3(0.0f, height + 0.001f, 0.0f));
    model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(width - 0.02f, 0.02f, length - 0.02f));
    shader.setMat4("model", model);
    bedWoodFlyweight->draw(shader);

    // === DOORS (front and back end walls) ===
    shader.setFloat("textureTiling", 1.2f);
    shader.setFloat("shininess", 2.0f);
    shader.setVec3("objectColor", glm::vec3(0.55f, 0.4f, 0.28f));

    const float openAngle = doorsOpen ? glm::radians(95.0f) : 0.0f;

    for (int side = -1; side <= 1; side += 2) {
        const float endX = side * (width * 0.5f + doorDepth * 0.45f);
        const float hingeZ = -doorWidth * 0.5f + 0.02f;
        const float centerFromHingeZ = doorWidth * 0.5f - 0.02f;
        const float openDir = side > 0 ? -1.0f : 1.0f;

        model = glm::mat4(1.0f);
        model = glm::translate(model, inst.position);
        model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(endX, doorHeight * 0.5f, hingeZ));
        model = glm::rotate(model, openDir * openAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, centerFromHingeZ));
        model = glm::scale(model, glm::vec3(doorDepth, doorHeight, doorWidth));
        shader.setMat4("model", model);
        doorFlyweight->draw(shader);
    }

    // === ROOF (single tri-prism roof body) ===
    shader.setFloat("textureTiling", 3.2f);
    shader.setFloat("shininess", 4.0f);

    const float roofOverhang = 1.1f;
    const float roofBaseY = height + 1.55f;

    model = glm::mat4(1.0f);
    model = glm::translate(model, inst.position + glm::vec3(0.0f, roofBaseY, 0.0f));
    model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(width + roofOverhang, 2.9f, length + roofOverhang * 1.5f));
    shader.setMat4("model", model);
    roofTriFlyweight->draw(shader);

    // Triangular gable ends
    shader.setFloat("textureTiling", 4.0f);
    shader.setFloat("shininess", 4.0f);
    shader.setVec3("objectColor", inst.isDark ? glm::vec3(0.3f, 0.22f, 0.18f) : glm::vec3(0.65f, 0.3f, 0.2f));

    const float gableHeight = 2.9f;
    const float gableDepth = 0.22f;

    for (int end = -1; end <= 1; end += 2) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, inst.position + glm::vec3(0.0f, height + gableHeight * 0.5f, 0.0f));
        model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, end * (length * 0.5f + 0.02f)));
        if (end < 0) {
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        model = glm::scale(model, glm::vec3(width - 0.2f, gableHeight, gableDepth));
        shader.setMat4("model", model);
        gableMesh->draw(shader);
    }

    // === INTERIOR: bunk beds ===
    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 0);
    shader.setFloat("textureTiling", 1.0f);
    shader.setFloat("shininess", 2.0f);

    const float bunkWidth = 2.2f;
    const float bunkLength = 4.8f;
    const float postThickness = 0.12f;
    const float bunkHeight = 5.2f;
    const float levelY[] = { 0.95f, 2.75f, 4.55f };
    const float bunkX[] = { -width * 0.33f, width * 0.33f };
    const float bunkZ[] = { -10.5f, -4.0f, 2.5f, 9.0f };

    for (float x : bunkX) {
        for (float z : bunkZ) {
            shader.setFloat("textureTiling", 0.9f);
            shader.setVec3("objectColor", glm::vec3(0.45f, 0.34f, 0.22f));

            for (int sx = -1; sx <= 1; sx += 2) {
                for (int sz = -1; sz <= 1; sz += 2) {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, inst.position + glm::vec3(0.0f, bunkHeight * 0.5f, 0.0f));
                    model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
                    model = glm::translate(model, glm::vec3(x + sx * (bunkWidth * 0.5f - postThickness), 0.0f, z + sz * (bunkLength * 0.5f - postThickness)));
                    model = glm::scale(model, glm::vec3(postThickness, bunkHeight, postThickness));
                    shader.setMat4("model", model);
                    bedWoodFlyweight->draw(shader);
                }
            }

            for (float y : levelY) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, inst.position + glm::vec3(0.0f, y, 0.0f));
                model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::translate(model, glm::vec3(x, 0.0f, z));
                model = glm::scale(model, glm::vec3(bunkWidth, 0.14f, bunkLength));
                shader.setMat4("model", model);
                bedWoodFlyweight->draw(shader);

                shader.setFloat("textureTiling", 1.8f);
                shader.setVec3("objectColor", glm::vec3(0.23f, 0.26f, 0.34f));
                model = glm::mat4(1.0f);
                model = glm::translate(model, inst.position + glm::vec3(0.0f, y + 0.18f, 0.0f));
                model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::translate(model, glm::vec3(x, 0.0f, z));
                model = glm::scale(model, glm::vec3(bunkWidth - 0.16f, 0.18f, bunkLength - 0.2f));
                shader.setMat4("model", model);
                bedMattressFlyweight->draw(shader);

                shader.setFloat("textureTiling", 0.9f);
                shader.setVec3("objectColor", glm::vec3(0.45f, 0.34f, 0.22f));
            }
        }
    }

    // === INTERIOR: hanging bulbs ===
    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 0);
    shader.setFloat("shininess", 12.0f);

    const float lampZ[] = { -10.5f, -4.0f, 2.5f, 9.0f };
    for (float z : lampZ) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, inst.position + glm::vec3(0.0f, height - 0.35f, 0.0f));
        model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, z));
        model = glm::scale(model, glm::vec3(1.4f, 0.12f, 1.4f));
        shader.setMat4("model", model);
        lampCanopyFlyweight->draw(shader);

        shader.setBool("useTexture", true);
        shader.setFloat("emissionStrength", 0.0f);
        shader.setVec3("objectColor", glm::vec3(0.7f, 0.72f, 0.75f));
        model = glm::mat4(1.0f);
        model = glm::translate(model, inst.position + glm::vec3(0.0f, height - 0.70f, 0.0f));
        model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, z));
        model = glm::scale(model, glm::vec3(0.44f, 0.48f, 0.44f));
        shader.setMat4("model", model);
        lampHolderFlyweight->draw(shader);

        shader.setBool("useTexture", false);
        shader.setFloat("emissionStrength", interiorLightsOn ? 1.85f : 0.02f);
        shader.setFloat("textureTiling", 1.0f);
        shader.setVec3("objectColor", interiorLightsOn ? glm::vec3(1.0f, 0.88f, 0.58f) : glm::vec3(0.30f, 0.26f, 0.2f));
        model = glm::mat4(1.0f);
        model = glm::translate(model, inst.position + glm::vec3(0.0f, height - 1.00f, 0.0f));
        model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, z));
        model = glm::scale(model, glm::vec3(0.26f, 0.26f, 0.26f));
        shader.setMat4("model", model);
        bulbFlyweight->draw(shader);

        shader.setBool("useTexture", true);
        shader.setFloat("emissionStrength", 0.0f);
    }
}

void BarrackGrid::setDoorsOpen(bool open) {
    doorsOpen = open;
}

void BarrackGrid::setInteriorLightsOn(bool on) {
    interiorLightsOn = on;
}

void BarrackGrid::render(Shader& shader) {
    for (const auto& inst : instances) {
        renderBarrack(shader, inst);
    }
}
