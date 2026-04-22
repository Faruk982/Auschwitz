#include "Environment.h"
#include "../primitives/Plane.h"
#include "../primitives/Cube.h"
#include "../primitives/Sphere.h"
#include "../Texture.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

void Environment::init(FlyweightFactory& factory) {
    // Ground
    if (!factory.get("ground")) {
        unsigned int tex = loadTexture("textures/gravel.png", true);
        factory.add("ground", Plane::generate(), tex);
    }
    groundFlyweight = factory.get("ground");

    // Roads
    if (!factory.get("road")) {
        unsigned int tex = loadTexture("textures/dirt_road.png", true);
        factory.add("road", Plane::generate(), tex);
    }
    roadFlyweight = factory.get("road");

    // Rails
    if (!factory.get("track_rail")) {
        unsigned int tex = loadTexture("textures/metal_iron.png", true);
        factory.add("track_rail", Cube::generate(), tex);
    }
    railFlyweight = factory.get("track_rail");

    // Sleepers
    if (!factory.get("track_sleeper")) {
        unsigned int tex = loadTexture("textures/wood_plank.png", true);
        factory.add("track_sleeper", Cube::generate(), tex);
    }
    sleeperFlyweight = factory.get("track_sleeper");

    // Lamp sphere head
    if (!factory.get("lamp_sphere")) {
        unsigned int tex = loadTexture("textures/Cone.jpg", true);
        factory.add("lamp_sphere", Sphere::generate(24, 24), tex);
    }
    lampSphereFlyweight = factory.get("lamp_sphere");
}

void Environment::render(Shader& shader) {
    // === GROUND PLANE ===
    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 1);
    shader.setFloat("textureBlendFactor", 0.82f);
    shader.setFloat("textureTiling", 25.0f);
    shader.setVec3("objectColor", 0.43f, 0.40f, 0.35f);
    shader.setFloat("shininess", 2.0f);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-40.0f, -0.01f, 5.0f));
    model = glm::scale(model, glm::vec3(540.0f, 1.0f, 210.0f));
    shader.setMat4("model", model);
    groundFlyweight->draw(shader);

    // === TRAIN RETURN LOOP EXTENSION ===
    shader.setInt("textureMode", 0);

    const float mainEndX = -200.0f;
    const float returnEndX = -80.0f;
    const float returnCenterZ = 12.0f;
    const float trackLen = returnEndX - mainEndX;
    const float midX = (returnEndX + mainEndX) * 0.5f;

    const float railHalf = 0.7f;
    const float railH = 0.14f;
    const float railW = 0.07f;

    const float sleeperW = 2.4f;
    const float sleeperH = 0.08f;
    const float sleeperL = 0.28f;

    auto bezier = [](const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) {
        float u = 1.0f - t;
        float uu = u * u;
        float tt = t * t;
        return (uu * u) * p0 + (3.0f * uu * t) * p1 + (3.0f * u * tt) * p2 + (tt * t) * p3;
    };

    auto drawSegmentX = [&](MeshFlyweight* fw, const glm::vec3& a, const glm::vec3& b, float y, float sy, float sz) {
        glm::vec3 d = b - a;
        float len = glm::length(d);
        if (len < 1e-4f) return;
        float yaw = std::atan2(d.z, d.x);
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, glm::vec3((a.x + b.x) * 0.5f, y, (a.z + b.z) * 0.5f));
        m = glm::rotate(m, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(len, sy, sz));
        shader.setMat4("model", m);
        fw->draw(shader);
    };

    auto drawSleeper = [&](const glm::vec3& p, const glm::vec3& tangent) {
        float yaw = std::atan2(tangent.x, tangent.z);
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, glm::vec3(p.x, sleeperH / 2.0f + 0.03f, p.z));
        m = glm::rotate(m, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(sleeperW, sleeperH, sleeperL));
        shader.setMat4("model", m);
        sleeperFlyweight->draw(shader);
    };

    // Ballast strip for return line
    shader.setFloat("textureTiling", 24.0f);
    shader.setFloat("shininess", 2.0f);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(midX, 0.02f, returnCenterZ));
    model = glm::scale(model, glm::vec3(trackLen, 1.0f, 4.0f));
    shader.setMat4("model", model);
    roadFlyweight->draw(shader);

    // Bézier-smoothed connectors
    const int curveSamples = 18;
    const float connectorBow = 9.0f;

    auto drawBezierConnector = [&](float cx, float sideSign) {
        glm::vec3 p0(cx, 0.0f, 0.0f);
        glm::vec3 p1(cx + sideSign * connectorBow, 0.0f, returnCenterZ * 0.2f);
        glm::vec3 p2(cx + sideSign * connectorBow, 0.0f, returnCenterZ * 0.8f);
        glm::vec3 p3(cx, 0.0f, returnCenterZ);

        // Ballast ribbon
        shader.setFloat("textureTiling", 24.0f);
        shader.setFloat("shininess", 2.0f);
        for (int i = 0; i < curveSamples; ++i) {
            float t0 = static_cast<float>(i) / static_cast<float>(curveSamples);
            float t1 = static_cast<float>(i + 1) / static_cast<float>(curveSamples);
            glm::vec3 a = bezier(p0, p1, p2, p3, t0);
            glm::vec3 b = bezier(p0, p1, p2, p3, t1);
            drawSegmentX(roadFlyweight, a, b, 0.02f, 1.0f, 4.0f);
        }

        // Rails
        shader.setFloat("textureTiling", 60.0f);
        shader.setFloat("shininess", 16.0f);
        for (int i = 0; i < curveSamples; ++i) {
            float t0 = static_cast<float>(i) / static_cast<float>(curveSamples);
            float t1 = static_cast<float>(i + 1) / static_cast<float>(curveSamples);
            glm::vec3 a = bezier(p0, p1, p2, p3, t0);
            glm::vec3 b = bezier(p0, p1, p2, p3, t1);

            glm::vec3 t = glm::normalize(b - a);
            glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), t));

            drawSegmentX(railFlyweight, a + right * railHalf, b + right * railHalf, railH / 2.0f + 0.05f, railH, railW);
            drawSegmentX(railFlyweight, a - right * railHalf, b - right * railHalf, railH / 2.0f + 0.05f, railH, railW);
        }

        // Sleepers
        shader.setFloat("textureTiling", 1.0f);
        shader.setFloat("shininess", 4.0f);
        const int sleeperSamples = 8;
        for (int i = 0; i <= sleeperSamples; ++i) {
            float t0 = static_cast<float>(i) / static_cast<float>(sleeperSamples);
            float t1 = glm::min(1.0f, t0 + 0.03f);
            glm::vec3 p = bezier(p0, p1, p2, p3, t0);
            glm::vec3 pNext = bezier(p0, p1, p2, p3, t1);
            drawSleeper(p, glm::normalize(pNext - p));
        }
    };

    drawBezierConnector(mainEndX, -1.0f);
    drawBezierConnector(returnEndX, 1.0f);

    // Rails for return line
    shader.setFloat("textureTiling", 60.0f);
    shader.setFloat("shininess", 16.0f);
    for (int side = -1; side <= 1; side += 2) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(midX, railH / 2 + 0.05f, returnCenterZ + side * railHalf));
        model = glm::scale(model, glm::vec3(trackLen, railH, railW));
        shader.setMat4("model", model);
        railFlyweight->draw(shader);
    }

    // Sleepers on return line
    shader.setFloat("textureTiling", 1.0f);
    shader.setFloat("shininess", 4.0f);
    for (float x = mainEndX; x <= returnEndX; x += 2.0f) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, sleeperH / 2 + 0.03f, returnCenterZ));
        model = glm::scale(model, glm::vec3(sleeperL, sleeperH, sleeperW));
        shader.setMat4("model", model);
        sleeperFlyweight->draw(shader);
    }

    // === PARALLEL SERVICE ROADS (between barrack lines) ===
    // Asymmetric placement keeps roads between barrack lanes on both sides.
    float serviceRoadZ[] = { -30.0f, 45.0f };
    shader.setInt("textureMode", 1);
    shader.setFloat("textureTiling", 18.0f);
    shader.setFloat("textureBlendFactor", 0.68f);

    for (float z : serviceRoadZ) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-50.0f, 0.01f, z));
        model = glm::scale(model, glm::vec3(300.0f, 1.0f, 4.0f));
        shader.setMat4("model", model);
        roadFlyweight->draw(shader);
    }

    // === CONNECTOR ROAD to Block 11 area ===
    shader.setFloat("textureTiling", 12.0f);
    shader.setFloat("textureBlendFactor", 0.70f);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.01f, 0.0f));
    model = glm::scale(model, glm::vec3(10.0f, 1.0f, 112.0f));
    shader.setMat4("model", model);
    roadFlyweight->draw(shader);

    // === SPOTLIGHT LAMP POSTS (visual source) ===
    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 0);
    shader.setFloat("textureTiling", 2.0f);
    shader.setFloat("shininess", 16.0f);

    const glm::vec3 spotPositions[4] = {
        glm::vec3(170.0f, 10.0f, -45.0f),
        glm::vec3(170.0f, 10.0f, 45.0f),
        glm::vec3(-245.0f, 10.0f, -75.0f),
        glm::vec3(-245.0f, 10.0f, 75.0f)
    };

    const glm::vec3 spotDirections[4] = {
        glm::normalize(glm::vec3(-0.7f, -0.65f, 0.15f)),
        glm::normalize(glm::vec3(-0.7f, -0.65f, -0.15f)),
        glm::normalize(glm::vec3(0.8f, -0.55f, 0.2f)),
        glm::normalize(glm::vec3(0.8f, -0.55f, -0.2f))
    };

    auto drawMetalBox = [&](const glm::vec3& p, const glm::vec3& s) {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, p);
        m = glm::scale(m, s);
        shader.setMat4("model", m);
        railFlyweight->draw(shader);
    };

    auto drawLampSphere = [&](const glm::vec3& p, float r) {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, p);
        m = glm::scale(m, glm::vec3(r));
        shader.setMat4("model", m);
        lampSphereFlyweight->draw(shader);
    };

    for (int i = 0; i < 4; ++i) {
        const glm::vec3& p = spotPositions[i];
        glm::vec3 h = glm::normalize(glm::vec3(spotDirections[i].x, 0.0f, spotDirections[i].z));

        drawMetalBox(glm::vec3(p.x, 5.0f, p.z), glm::vec3(0.25f, 10.0f, 0.25f));
        drawMetalBox(glm::vec3(p.x, 10.1f, p.z), glm::vec3(0.6f, 0.2f, 0.6f));
        drawMetalBox(glm::vec3(p.x + h.x * 0.65f, 9.95f, p.z + h.z * 0.65f), glm::vec3(1.0f, 0.14f, 0.22f));

        shader.setBool("useTexture", false);
        shader.setInt("textureMode", 0);
        shader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);
        shader.setFloat("shininess", 2.0f);
        shader.setBool("emissiveOnly", true);
        shader.setFloat("emissionStrength", 2.2f);
        drawLampSphere(glm::vec3(p.x + h.x * 1.15f, 9.72f, p.z + h.z * 1.15f), 0.29f);
        shader.setBool("emissiveOnly", false);
        shader.setFloat("emissionStrength", 0.015f);
        shader.setBool("useTexture", true);
        shader.setInt("textureMode", 0);
    }
}
