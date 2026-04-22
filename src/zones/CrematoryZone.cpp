#include "CrematoryZone.h"
#include "../primitives/Cube.h"
#include "../primitives/Cylinder.h"
#include "../primitives/TriPrism.h"
#include "../primitives/Cone.h"
#include "../primitives/Sphere.h"
#include "../Texture.h"
#include <glm/gtc/matrix_transform.hpp>

void CrematoryZone::init(FlyweightFactory& factory) {
    if (!factory.get("dark_brick")) {
        unsigned int tex = loadTexture("textures/brick_dark.png", true);
        factory.add("dark_brick", Cube::generate(), tex);
    }
    darkBrickFlyweight = factory.get("dark_brick");

    if (!factory.get("chimney")) {
        unsigned int tex = loadTexture("textures/brick_dark.png", true);
        factory.add("chimney", Cylinder::generate(12), tex);
    }
    chimneyFlyweight = factory.get("chimney");

    if (!factory.get("wood_structure")) {
        unsigned int tex = loadTexture("textures/wood_plank.png", true);
        factory.add("wood_structure", Cube::generate(), tex);
    }
    woodFlyweight = factory.get("wood_structure");

    if (!factory.get("crem_roof_tri")) {
        unsigned int tex = loadTexture("textures/roof_tile.png", true);
        factory.add("crem_roof_tri", TriPrism::generate(), tex);
    }
    cremRoofTriFlyweight = factory.get("crem_roof_tri");

    if (!factory.get("crem_crate")) {
        unsigned int tex = loadTexture("textures/container2.png", true);
        factory.add("crem_crate", Cube::generate(), tex);
    }
    crateFlyweight = factory.get("crem_crate");

    if (!factory.get("crem_lamp_canopy")) {
        unsigned int tex = loadTexture("textures/metal_grey.png", true);
        factory.add("crem_lamp_canopy", Cylinder::generate(16), tex);
    }
    lampCanopyFlyweight = factory.get("crem_lamp_canopy");

    if (!factory.get("crem_lamp_holder")) {
        unsigned int tex = loadTexture("textures/metal_grey.png", true);
        factory.add("crem_lamp_holder", Cone::generate(20), tex);
    }
    lampHolderFlyweight = factory.get("crem_lamp_holder");

    if (!factory.get("crem_bulb")) {
        unsigned int tex = loadTexture("textures/metal_grey.png", true);
        factory.add("crem_bulb", Sphere::generate(16, 16), tex);
    }
    bulbFlyweight = factory.get("crem_bulb");
}

void CrematoryZone::render(Shader& shader) {
    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 0);
    shader.setFloat("textureTiling", 3.0f);
    shader.setFloat("shininess", 4.0f);

    // Crematoria behind Block 11: 2x2 on each side of the main road (8 total)
    const float cremX[] = { -220.0f, -255.0f };
    const float cremZLeft[] = { -24.0f, -48.0f };
    const float cremZRight[] = { 24.0f, 48.0f };

    auto drawInterior = [&](float x, float z) {
        glm::mat4 model = glm::mat4(1.0f);

        // Thin interior floor slab
        shader.setFloat("textureTiling", 2.0f);
        model = glm::translate(model, glm::vec3(x, 0.08f, z));
        model = glm::scale(model, glm::vec3(18.5f, 0.16f, 8.5f));
        shader.setMat4("model", model);
        darkBrickFlyweight->draw(shader);

        // Roof support beams
        shader.setFloat("textureTiling", 1.0f);
        for (int i = -2; i <= 2; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x + i * 3.5f, 4.35f, z));
            model = glm::scale(model, glm::vec3(0.22f, 0.22f, 8.2f));
            shader.setMat4("model", model);
            woodFlyweight->draw(shader);
        }

        // Two simple benches
        const float benchZ[2] = { -2.0f, 2.0f };
        for (float bz : benchZ) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, 0.62f, z + bz));
            model = glm::scale(model, glm::vec3(6.0f, 0.18f, 1.0f));
            shader.setMat4("model", model);
            woodFlyweight->draw(shader);

            const float legX[2] = { -2.5f, 2.5f };
            for (float lx : legX) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x + lx, 0.31f, z + bz));
                model = glm::scale(model, glm::vec3(0.16f, 0.62f, 0.16f));
                shader.setMat4("model", model);
                woodFlyweight->draw(shader);
            }
        }

        // Storage crates near back wall
        shader.setFloat("textureTiling", 1.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x - 7.0f, 0.45f, z - 3.1f));
        model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
        shader.setMat4("model", model);
        crateFlyweight->draw(shader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x - 5.8f, 0.45f, z - 3.1f));
        model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
        shader.setMat4("model", model);
        crateFlyweight->draw(shader);

        // Hanging bulbs (visible ON/OFF state for point lights)
        shader.setFloat("shininess", 10.0f);
        const float lampX[] = { -4.2f, 4.2f };
        for (float lx : lampX) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x + lx, 4.55f, z));
            model = glm::scale(model, glm::vec3(0.9f, 0.1f, 0.9f));
            shader.setMat4("model", model);
            lampCanopyFlyweight->draw(shader);

            shader.setBool("useTexture", true);
            shader.setFloat("emissionStrength", 0.0f);
            shader.setVec3("objectColor", glm::vec3(0.7f, 0.72f, 0.75f));
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x + lx, 4.34f, z));
            model = glm::scale(model, glm::vec3(0.34f, 0.38f, 0.34f));
            shader.setMat4("model", model);
            lampHolderFlyweight->draw(shader);

            shader.setBool("useTexture", false);
            shader.setFloat("emissionStrength", interiorLightsOn ? 1.85f : 0.02f);
            shader.setVec3("objectColor", interiorLightsOn ? glm::vec3(1.0f, 0.88f, 0.58f) : glm::vec3(0.30f, 0.26f, 0.2f));
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x + lx, 4.10f, z));
            model = glm::scale(model, glm::vec3(0.16f, 0.16f, 0.16f));
            shader.setMat4("model", model);
            bulbFlyweight->draw(shader);

            shader.setBool("useTexture", true);
            shader.setFloat("emissionStrength", 0.0f);
        }

        shader.setFloat("shininess", 4.0f);
    };

    auto drawCrematorium = [&](float x, float z, float sideSign) {
        glm::mat4 model = glm::mat4(1.0f);

        const float buildingWidth = 20.0f;
        const float buildingHeight = 5.0f;
        const float buildingDepth = 10.0f;
        const float wallThickness = 0.28f;

        // Front double-door (facing road)
        const float doorHeight = 3.2f;
        const float doorDepth = 0.22f;
        const float leafWidth = 1.55f;
        const float leafOffsetX = 0.85f;
        const float doorOpeningWidth = leafWidth * 2.0f + 0.2f;

        auto drawWallSegment = [&](const glm::vec3& localOffset, const glm::vec3& localScale, float tiling = 3.0f) {
            shader.setFloat("textureTiling", tiling);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, buildingHeight * 0.5f, z));
            model = glm::translate(model, localOffset);
            model = glm::scale(model, localScale);
            shader.setMat4("model", model);
            darkBrickFlyweight->draw(shader);
        };

        // Main walls with real front doorway opening
        shader.setFloat("shininess", 4.0f);
        shader.setVec3("objectColor", glm::vec3(1.0f));

        // Side walls
        drawWallSegment(
            glm::vec3(-buildingWidth * 0.5f + wallThickness * 0.5f, 0.0f, 0.0f),
            glm::vec3(wallThickness, buildingHeight, buildingDepth),
            3.0f
        );
        drawWallSegment(
            glm::vec3(buildingWidth * 0.5f - wallThickness * 0.5f, 0.0f, 0.0f),
            glm::vec3(wallThickness, buildingHeight, buildingDepth),
            3.0f
        );

        // Back wall (full)
        drawWallSegment(
            glm::vec3(0.0f, 0.0f, sideSign * (buildingDepth * 0.5f - wallThickness * 0.5f)),
            glm::vec3(buildingWidth, buildingHeight, wallThickness),
            3.0f
        );

        // Front wall split around door opening
        const float frontWallZ = -sideSign * (buildingDepth * 0.5f - wallThickness * 0.5f);
        const float sideWallWidth = (buildingWidth - doorOpeningWidth) * 0.5f;
        const float sideWallOffsetX = (doorOpeningWidth + sideWallWidth) * 0.5f;
        const float topWallHeight = buildingHeight - doorHeight;

        drawWallSegment(
            glm::vec3(-sideWallOffsetX, 0.0f, frontWallZ),
            glm::vec3(sideWallWidth, buildingHeight, wallThickness),
            3.0f
        );
        drawWallSegment(
            glm::vec3(sideWallOffsetX, 0.0f, frontWallZ),
            glm::vec3(sideWallWidth, buildingHeight, wallThickness),
            3.0f
        );
        drawWallSegment(
            glm::vec3(0.0f, doorHeight * 0.5f, frontWallZ),
            glm::vec3(doorOpeningWidth, topWallHeight, wallThickness),
            3.0f * (topWallHeight / buildingHeight)
        );

        // Front double-door (hinged)
        shader.setFloat("textureTiling", 1.2f);
        shader.setFloat("shininess", 2.0f);
        shader.setVec3("objectColor", glm::vec3(0.55f, 0.4f, 0.28f));

        const float frontZ = z - sideSign * (buildingDepth * 0.5f + doorDepth * 0.5f + 0.02f);
        const float hingeInset = 0.03f;
        const float openAngle = doorsOpen ? glm::radians(95.0f) : 0.0f;

        // Left leaf
        {
            const float hingeX = x - (leafOffsetX + leafWidth * 0.5f - hingeInset);
            const float centerFromHingeX = leafWidth * 0.5f - hingeInset;
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(hingeX, doorHeight * 0.5f, frontZ));
            model = glm::rotate(model, sideSign * openAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::translate(model, glm::vec3(centerFromHingeX, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(leafWidth, doorHeight, doorDepth));
            shader.setMat4("model", model);
            woodFlyweight->draw(shader);
        }

        // Right leaf
        {
            const float hingeX = x + (leafOffsetX + leafWidth * 0.5f - hingeInset);
            const float centerFromHingeX = -(leafWidth * 0.5f - hingeInset);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(hingeX, doorHeight * 0.5f, frontZ));
            model = glm::rotate(model, -sideSign * openAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::translate(model, glm::vec3(centerFromHingeX, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(leafWidth, doorHeight, doorDepth));
            shader.setMat4("model", model);
            woodFlyweight->draw(shader);
        }

        // Tri-prism roof
        shader.setFloat("textureTiling", 3.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, 6.2f, z));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(11.0f, 2.4f, 21.5f));
        shader.setMat4("model", model);
        cremRoofTriFlyweight->draw(shader);

        // Chimney
        shader.setFloat("textureTiling", 2.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x + 5.0f, 7.0f, z));
        model = glm::scale(model, glm::vec3(1.5f, 14.0f, 1.5f));
        shader.setMat4("model", model);
        chimneyFlyweight->draw(shader);

        drawInterior(x, z);

        // Gallows near each crematorium (offset away from road)
        shader.setFloat("textureTiling", 1.0f);
        float gallowsX = x + 15.0f;
        float gallowsZ = z + sideSign * 8.0f;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(gallowsX - 1.0f, 2.0f, gallowsZ));
        model = glm::scale(model, glm::vec3(0.2f, 4.0f, 0.2f));
        shader.setMat4("model", model);
        woodFlyweight->draw(shader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(gallowsX + 1.0f, 2.0f, gallowsZ));
        model = glm::scale(model, glm::vec3(0.2f, 4.0f, 0.2f));
        shader.setMat4("model", model);
        woodFlyweight->draw(shader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(gallowsX, 4.0f, gallowsZ));
        model = glm::scale(model, glm::vec3(2.5f, 0.2f, 0.2f));
        shader.setMat4("model", model);
        woodFlyweight->draw(shader);
    };

    for (float x : cremX) {
        for (float z : cremZLeft) {
            drawCrematorium(x, z, -1.0f);
        }
        for (float z : cremZRight) {
            drawCrematorium(x, z, 1.0f);
        }
    }
}

void CrematoryZone::setDoorsOpen(bool open) {
    doorsOpen = open;
}

void CrematoryZone::setInteriorLightsOn(bool on) {
    interiorLightsOn = on;
}
