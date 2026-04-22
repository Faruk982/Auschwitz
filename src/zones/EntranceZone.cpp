#include "EntranceZone.h"
#include "../primitives/Cube.h"
#include "../primitives/Cylinder.h"
#include "../primitives/TriPrism.h"
#include "../primitives/Plane.h"
#include "../Texture.h"
#include <glm/gtc/matrix_transform.hpp>

// ============================================================
// Layout constants for the Birkenau "Gate of Death"
// ============================================================
static const float GX          = 175.0f;   // gate X position (fence line)
static const float DEPTH       = 8.0f;    // building depth (X)
static const float WING_H      = 5.5f;    // wing wall height
static const float ARCH_HALF   = 4.0f;    // half-width of central archway (Z)

// Secondary passage in south wing
static const float SEC_Z       = -30.0f;  // centre Z
static const float SEC_W       = 6.0f;    // passage width

static const float TRACK_MIN_X = -200.0f;
static const float TRACK_MAX_X = 200.0f;
static const float TRAIN_SPEED = 28.0f;
static const float TRAIN_MAX_LATERAL = 0.8f;

// ============================================================
// Initialisation — register flyweights
// ============================================================
void EntranceZone::init(FlyweightFactory& factory) {
    auto reg = [&](const char* key, auto meshFn, const char* tex) {
        if (!factory.get(key)) factory.add(key, meshFn(), loadTexture(tex, true));
        return factory.get(key);
    };

    gateRedBrick      = reg("gate_redbrick",   Cube::generate,           "textures/brick_red.jpg");
    gatehouseFlyweight= reg("gatehouse",       Cube::generate,           "textures/brick_dark.png");
    roofFlyweight     = reg("gate_roof",       Cube::generate,           "textures/roof_tile.png");
    roofPrismFlyweight= reg("gate_roof_prism", TriPrism::generate,       "textures/roof_tile.png");
    windowFlyweight   = reg("gate_window",     Cube::generate,           "textures/concrete.png");
    railFlyweight     = reg("track_rail",      Cube::generate,           "textures/metal_iron.png");
    sleeperFlyweight  = reg("track_sleeper",   Cube::generate,           "textures/wood_plank.png");
    trainBodyFlyweight= reg("train_body",      Cube::generate,           "textures/wood_plank.png");
    trainRoofFlyweight= reg("train_roof",      TriPrism::generate,       "textures/Cone.jpg");
    trainWheelFlyweight=reg("train_wheel",     []{ return Cylinder::generate(12); }, "textures/metal_grey.png");
    trainFrontFlyweight=reg("train_front",     Cube::generate,           "textures/container2.png");
    roadFlyweight     = reg("entrance_road",   Plane::generate,          "textures/dirt_road.png");
}

// ============================================================
void EntranceZone::render(Shader& shader) {
    renderTrainTracks(shader);
    renderTrainCars(shader);
    renderGatehouse(shader);
    renderLeftEntrance(shader);
}

void EntranceZone::updateTrainMovement(float forwardInput, float lateralInput, float deltaTime) {
    const float moveX = -forwardInput * TRAIN_SPEED * deltaTime;

    trainOffsetX += moveX;

    const float carLen = 8.0f;
    const float stride = carLen + 1.0f;
    const float frontCarBaseX = 192.0f;
    const float lastCarBaseX = frontCarBaseX - 3.0f * stride;

    const float localMaxX = frontCarBaseX + carLen * 0.5f;
    const float localMinX = lastCarBaseX - carLen * 0.5f - 1.55f;

    trainOffsetX = glm::clamp(trainOffsetX, TRACK_MIN_X - localMinX, TRACK_MAX_X - localMaxX);
    trainOffsetZ = 0.0f;
}

// ============================================================
// GATEHOUSE — extended Birkenau facade with central arch
// ============================================================
void EntranceZone::renderGatehouse(Shader& shader) {
    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 0);
    shader.setFloat("textureTiling", 6.0f);
    shader.setFloat("shininess", 4.0f);

    glm::mat4 m;

    // Helper: draw a cube at position p with size s using given flyweight
    auto box = [&](MeshFlyweight* fw, glm::vec3 p, glm::vec3 s) {
        m = glm::translate(glm::mat4(1.0f), p);
        m = glm::scale(m, s);
        shader.setMat4("model", m);
        fw->draw(shader);
    };

    // ----------------------------------------------------------
    // SOUTH WING — split for secondary passage
    // Part A (outer): Z from -50 to -33
    // Part B (inner): Z from -27 to -4
    // ----------------------------------------------------------
    float sA_z0 = -50.0f, sA_z1 = SEC_Z - SEC_W / 2.0f;       // -50 to -33
    float sB_z0 = SEC_Z + SEC_W / 2.0f, sB_z1 = -ARCH_HALF;   // -27 to -4
    float sA_len = sA_z1 - sA_z0;                               // 17
    float sB_len = sB_z1 - sB_z0;                               // 23

    box(gateRedBrick, {GX, WING_H/2, (sA_z0+sA_z1)/2}, {DEPTH, WING_H, sA_len});
    box(gateRedBrick, {GX, WING_H/2, (sB_z0+sB_z1)/2}, {DEPTH, WING_H, sB_len});

    // Lintel over secondary passage
    box(gateRedBrick, {GX, WING_H - 0.4f, SEC_Z}, {DEPTH, 0.8f, SEC_W});

    // ----------------------------------------------------------
    // NORTH WING: Z from +4 to +50
    // ----------------------------------------------------------
    float nLen = 46.0f;
    box(gateRedBrick, {GX, WING_H/2, 27.0f}, {DEPTH, WING_H, nLen});

    // ----------------------------------------------------------
    // CENTRAL ARCHWAY — lintel beam above the opening
    // ----------------------------------------------------------
    float beamH = 1.0f;
    box(gateRedBrick, {GX, WING_H + beamH/2, 0.0f}, {DEPTH, beamH, ARCH_HALF*2});

    // ----------------------------------------------------------
    // WATCHTOWER above archway
    // ----------------------------------------------------------
    float tBase = WING_H + beamH;
    float tH    = 8.5f;
    float tW    = 10.0f;
    box(gateRedBrick, {GX, tBase + tH/2, 0.0f}, {DEPTH, tH, tW});

    // ----------------------------------------------------------
    // WING ROOFS (TriPrism — ridge runs along Z by default)
    // ----------------------------------------------------------
    shader.setFloat("textureTiling", 3.0f);
    float roofH = 2.2f, oh = 1.5f;

    // south-A roof
    m = glm::translate(glm::mat4(1.0f), glm::vec3(GX, WING_H + roofH/2, (sA_z0+sA_z1)/2));
    m = glm::scale(m, glm::vec3(DEPTH + oh, roofH, sA_len + oh));
    shader.setMat4("model", m);
    roofPrismFlyweight->draw(shader);

    // south-B roof
    m = glm::translate(glm::mat4(1.0f), glm::vec3(GX, WING_H + roofH/2, (sB_z0+sB_z1)/2));
    m = glm::scale(m, glm::vec3(DEPTH + oh, roofH, sB_len + oh));
    shader.setMat4("model", m);
    roofPrismFlyweight->draw(shader);

    // north wing roof
    m = glm::translate(glm::mat4(1.0f), glm::vec3(GX, WING_H + roofH/2, 27.0f));
    m = glm::scale(m, glm::vec3(DEPTH + oh, roofH, nLen + oh));
    shader.setMat4("model", m);
    roofPrismFlyweight->draw(shader);

    // ----------------------------------------------------------
    // WATCHTOWER ROOF
    // ----------------------------------------------------------
    shader.setFloat("textureTiling", 2.0f);
    float tTop = tBase + tH;
    float tRoofH = 3.5f;

    m = glm::translate(glm::mat4(1.0f), glm::vec3(GX, tTop + tRoofH/2, 0.0f));
    m = glm::scale(m, glm::vec3(DEPTH + 2.5f, tRoofH, tW + 2.5f));
    shader.setMat4("model", m);
    roofPrismFlyweight->draw(shader);

    // Eave slab under tower roof for visual weight
    box(roofFlyweight, {GX, tTop + 0.15f, 0.0f}, {DEPTH + 3.0f, 0.3f, tW + 3.0f});

    // ----------------------------------------------------------
    // WATCHTOWER WINDOWS (dark concrete insets)
    // ----------------------------------------------------------
    shader.setFloat("textureTiling", 1.0f);
    shader.setFloat("shininess", 2.0f);

    float winY = tBase + tH * 0.55f;

    // Front & back faces (±X)
    for (int s = -1; s <= 1; s += 2)
        for (int w = -1; w <= 1; w++)
            box(windowFlyweight,
                {GX + s*(DEPTH/2 + 0.06f), winY, w*2.8f},
                {0.15f, 2.0f, 1.2f});

    // Side faces (±Z)
    for (int s = -1; s <= 1; s += 2)
        for (int w = -1; w <= 1; w++)
            box(windowFlyweight,
                {GX + w*2.0f, winY, s*(tW/2 + 0.06f)},
                {1.2f, 2.0f, 0.15f});

    // ----------------------------------------------------------
    // WING WINDOWS (small brick-inset rectangles on front face)
    // ----------------------------------------------------------
    shader.setFloat("textureTiling", 1.0f);

    // South wing windows (on +X face, evenly spaced)
    for (float z = -48.0f; z <= -6.0f; z += 5.0f) {
        // Skip the passage gap
        if (z > SEC_Z - SEC_W/2 - 1.0f && z < SEC_Z + SEC_W/2 + 1.0f) continue;
        box(windowFlyweight,
            {GX + DEPTH/2 + 0.06f, WING_H * 0.6f, z},
            {0.12f, 1.4f, 1.0f});
    }
    // North wing windows
    for (float z = 6.0f; z <= 48.0f; z += 5.0f) {
        box(windowFlyweight,
            {GX + DEPTH/2 + 0.06f, WING_H * 0.6f, z},
            {0.12f, 1.4f, 1.0f});
    }
}

// ============================================================
// TRAIN TRACKS — parallel steel rails + wooden sleepers
// ============================================================
void EntranceZone::renderTrainTracks(Shader& shader) {
    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 0);

    glm::mat4 m;
    float trackLen = 400.0f;    // from X=200 to X=-200
    float midX     = 0.0f;     // centre of the track span

    // --- Ballast bed (gravel strip under tracks) ---
    shader.setFloat("textureTiling", 30.0f);
    shader.setFloat("shininess", 2.0f);

    m = glm::translate(glm::mat4(1.0f), glm::vec3(midX, 0.02f, 0.0f));
    m = glm::scale(m, glm::vec3(trackLen, 1.0f, 4.0f));
    shader.setMat4("model", m);
    roadFlyweight->draw(shader);   // reuse dirt texture for ballast bed

    // --- Steel rails (two long thin boxes) ---
    shader.setFloat("textureTiling", 60.0f);
    shader.setFloat("shininess", 16.0f);

    float railHalf = 0.7f;    // half the gauge
    float railH    = 0.14f;
    float railW    = 0.07f;

    for (int side = -1; side <= 1; side += 2) {
        m = glm::translate(glm::mat4(1.0f), glm::vec3(midX, railH/2 + 0.05f, side * railHalf));
        m = glm::scale(m, glm::vec3(trackLen, railH, railW));
        shader.setMat4("model", m);
        railFlyweight->draw(shader);
    }

    // --- Wooden sleepers (ties) ---
    shader.setFloat("textureTiling", 1.0f);
    shader.setFloat("shininess", 4.0f);

    float sleeperW = 2.4f;     // Z size
    float sleeperH = 0.08f;
    float sleeperL = 0.28f;    // X size

    for (float x = -200.0f; x <= 200.0f; x += 2.0f) {
        m = glm::translate(glm::mat4(1.0f), glm::vec3(x, sleeperH/2 + 0.03f, 0.0f));
        m = glm::scale(m, glm::vec3(sleeperL, sleeperH, sleeperW));
        shader.setMat4("model", m);
        sleeperFlyweight->draw(shader);
    }
}

// ============================================================
// TRAIN CARS — cattle wagons on the tracks
// ============================================================
void EntranceZone::renderTrainCars(Shader& shader) {
    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 0);

    glm::mat4 m;

    float carLen   = 8.0f;
    float carW     = 3.5f;
    float carH     = 3.0f;
    float baseY    = 0.7f;     // bottom of body above ground
    float roofH    = 1.3f;
    float stride   = carLen + 1.0f;

    // 4 wagons: first at X=192 (outside gate), last at X=165 (inside)
    for (int i = 0; i < 4; i++) {
        float cx = 192.0f - i * stride + trainOffsetX;
        float bodyMidY = baseY + carH / 2.0f;
        float cz = trainOffsetZ;

        // --- Wagon body (wood planks) ---
        shader.setFloat("textureTiling", 2.0f);
        shader.setFloat("shininess", 4.0f);

        m = glm::translate(glm::mat4(1.0f), glm::vec3(cx, bodyMidY, cz));
        m = glm::scale(m, glm::vec3(carLen, carH, carW));
        shader.setMat4("model", m);
        trainBodyFlyweight->draw(shader);

        // --- Peaked roof (TriPrism, ridge along X) ---
        shader.setFloat("textureTiling", 1.5f);

        m = glm::translate(glm::mat4(1.0f), glm::vec3(cx, baseY + carH + roofH/2, cz));
        m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(carW + 0.3f, roofH, carLen + 0.4f));
        shader.setMat4("model", m);
        trainRoofFlyweight->draw(shader);

        // --- Front detailing on last car (locomotive-like nose blocks) ---
        if (i == 3) {
            shader.setFloat("textureTiling", 1.4f);
            shader.setFloat("shininess", 5.0f);

            // Thin textured face panel on the true train front side
            m = glm::translate(glm::mat4(1.0f), glm::vec3(cx - carLen/2 - 0.07f, bodyMidY, cz));
            m = glm::scale(m, glm::vec3(0.14f, carH, carW));
            shader.setMat4("model", m);
            trainFrontFlyweight->draw(shader);

            // Lower front buffer block (front side)
            m = glm::translate(glm::mat4(1.0f), glm::vec3(cx - carLen/2 - 0.9f, 1.0f, cz));
            m = glm::scale(m, glm::vec3(1.2f, 0.9f, carW - 0.5f));
            shader.setMat4("model", m);
            trainFrontFlyweight->draw(shader);

            // Upper front cab block (front side)
            m = glm::translate(glm::mat4(1.0f), glm::vec3(cx - carLen/2 - 0.45f, 2.0f, cz));
            m = glm::scale(m, glm::vec3(0.9f, 1.0f, carW - 1.1f));
            shader.setMat4("model", m);
            trainFrontFlyweight->draw(shader);

            // Metal bumper strip
            shader.setFloat("textureTiling", 1.0f);
            shader.setFloat("shininess", 10.0f);
            m = glm::translate(glm::mat4(1.0f), glm::vec3(cx - carLen/2 - 1.55f, 0.6f, cz));
            m = glm::scale(m, glm::vec3(0.25f, 0.25f, carW - 0.7f));
            shader.setMat4("model", m);
            railFlyweight->draw(shader);
        }

        // --- Undercarriage / wheel bogies ---
        shader.setFloat("textureTiling", 1.0f);
        shader.setFloat("shininess", 8.0f);

        for (int end = -1; end <= 1; end += 2) {
            // Bogie block
            m = glm::translate(glm::mat4(1.0f),
                glm::vec3(cx + end * (carLen/2 - 1.2f), 0.35f, cz));
            m = glm::scale(m, glm::vec3(1.6f, 0.5f, carW - 0.4f));
            shader.setMat4("model", m);
            railFlyweight->draw(shader);   // dark metal texture

            // Wheels (small cylinders on each side)
            for (int side = -1; side <= 1; side += 2) {
                m = glm::translate(glm::mat4(1.0f),
                    glm::vec3(cx + end*(carLen/2 - 1.2f), 0.3f, cz + side*(carW/2 - 0.1f)));
                m = glm::rotate(m, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                m = glm::scale(m, glm::vec3(0.55f, 0.2f, 0.55f));
                shader.setMat4("model", m);
                trainWheelFlyweight->draw(shader);
            }
        }
    }
}

// ============================================================
// LEFT ENTRANCE — secondary road & smaller gate
// ============================================================
void EntranceZone::renderLeftEntrance(Shader& shader) {
    shader.setBool("useTexture", true);
    shader.setInt("textureMode", 1);
    shader.setFloat("textureBlendFactor", 0.72f);
    shader.setFloat("textureTiling", 12.0f);
    shader.setFloat("shininess", 2.0f);
    shader.setVec3("objectColor", 0.50f, 0.44f, 0.36f);

    glm::mat4 m;
    const float entranceRoadW = 4.0f;

    // Road surface (runs along X through the passage in the south wing)
    m = glm::translate(glm::mat4(1.0f), glm::vec3(GX, 0.03f, SEC_Z));
    m = glm::scale(m, glm::vec3(60.0f, 1.0f, entranceRoadW));
    shader.setMat4("model", m);
    roadFlyweight->draw(shader);

    // Extend road further into camp
    m = glm::translate(glm::mat4(1.0f), glm::vec3(GX - 50.0f, 0.03f, SEC_Z));
    m = glm::scale(m, glm::vec3(160.0f, 1.0f, entranceRoadW));
    shader.setMat4("model", m);
    roadFlyweight->draw(shader);

    // --- Small gate pillars flanking the passage ---
    shader.setInt("textureMode", 0);
    shader.setFloat("textureTiling", 3.0f);
    shader.setFloat("shininess", 4.0f);

    float pillarH = WING_H + 0.5f;
    for (int s = -1; s <= 1; s += 2) {
        m = glm::translate(glm::mat4(1.0f),
            glm::vec3(GX, pillarH/2, SEC_Z + s*(SEC_W/2 + 0.3f)));
        m = glm::scale(m, glm::vec3(DEPTH + 0.4f, pillarH, 0.6f));
        shader.setMat4("model", m);
        gatehouseFlyweight->draw(shader);
    }
}
