#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Shader.h"
#include "../Camera.h"
#include "Scene.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera — start at the gate facing into the camp (-X direction)
Camera camera(glm::vec3(120.0f, 2.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 180.0f, 0.0f);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool mouseCaptured = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Toggles
bool enableDirLight = true;
bool enablePointLight = true;
bool enableSpotLight = true;
bool enableAmbient = true;
bool enableDiffuse = true;
bool enableSpecular = true;
bool wireframeMode = false;
bool keyFPressed = false;
bool key1Pressed = false;
bool key2Pressed = false;
bool key3Pressed = false;
bool key4Pressed = false;
bool key5Pressed = false;
bool key6Pressed = false;
bool key7Pressed = false;
bool key8Pressed = false;
bool keyVPressed = false;
bool keyMPressed = false;
bool leftMousePressed = false;
bool barrackDoorsOpen = false;
bool quadViewMode = false;
int textureDisplayMode = 2; // 0=without texture, 1=with texture, 2=blended texture

// Camera roam bounds (expanded for full gate and wider scene coverage)
const float CAMERA_MIN_X = -360.0f;
const float CAMERA_MAX_X = 340.0f;
const float CAMERA_MIN_Z = -180.0f;
const float CAMERA_MAX_Z = 180.0f;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Auschwitz I — Educational 3D Reconstruction", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE); // Enable MSAA

    // Camera speed for large scene
    camera.MovementSpeed = 15.0f;

    // Build shaders
    Shader phongShader("E:/4-2/LAB/Graphics/Project_Auschwitz/shaders/phong.vert", "E:/4-2/LAB/Graphics/Project_Auschwitz/shaders/phong.frag");

    // Init Scene
    Scene scene;
    scene.init();

    phongShader.use();
    phongShader.setInt("texture1", 0);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        scene.setBarrackDoorsOpen(barrackDoorsOpen);
        scene.setInteriorLightsOn(enablePointLight);

        float trainForward = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) trainForward += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) trainForward -= 1.0f;
        scene.updateTrainMovement(trainForward, 0.0f, deltaTime);

        // Sky blue background (daytime default)
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (wireframeMode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        phongShader.use();

        // === LIGHTING SETUP ===
        phongShader.setBool("enableDirLight", enableDirLight);
        phongShader.setBool("enablePointLight", enablePointLight);
        phongShader.setBool("enableSpotLight", enableSpotLight);
        phongShader.setBool("enableAmbient", enableAmbient);
        phongShader.setBool("enableDiffuse", enableDiffuse);
        phongShader.setBool("enableSpecular", enableSpecular);
        phongShader.setInt("textureDisplayMode", textureDisplayMode);

        // Sun — warm white directional light from above-left
        phongShader.setVec3("dirLight.direction", -0.3f, -0.8f, -0.2f);
        phongShader.setVec3("dirLight.ambient", 0.34f, 0.32f, 0.29f);
        phongShader.setVec3("dirLight.diffuse", 0.85f, 0.8f, 0.75f);
        phongShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.45f);

        // Default material
        phongShader.setFloat("shininess", 32.0f);
        phongShader.setFloat("emissionStrength", 0.015f);
        phongShader.setBool("emissiveOnly", false);

        // Point lights (in barrack cone-lamp region, with bigger radius)
        const glm::vec3 pointPositions[4] = {
            glm::vec3(71.5f, 7.0f, 32.0f),
            glm::vec3(78.0f, 7.0f, 32.0f),
            glm::vec3(84.5f, 7.0f, 32.0f),
            glm::vec3(91.0f, 7.0f, 32.0f)
        };

        for (int i = 0; i < 4; i++) {
            std::string n = std::to_string(i);
            phongShader.setVec3("pointLights[" + n + "].position", pointPositions[i]);
            phongShader.setVec3("pointLights[" + n + "].ambient", 0.05f, 0.045f, 0.04f);
            phongShader.setVec3("pointLights[" + n + "].diffuse", 0.62f, 0.57f, 0.50f);
            phongShader.setVec3("pointLights[" + n + "].specular", 0.42f, 0.38f, 0.34f);
            phongShader.setFloat("pointLights[" + n + "].constant", 1.0f);
            phongShader.setFloat("pointLights[" + n + "].linear", 0.020f);
            phongShader.setFloat("pointLights[" + n + "].quadratic", 0.0015f);
        }

        // Spotlights on static headlight posts (not camera-attached)
        const glm::vec3 spotPositions[4] = {
            glm::vec3(170.0f, 10.0f, -45.0f),
            glm::vec3(170.0f, 10.0f,  45.0f),
            glm::vec3(-245.0f, 10.0f, -75.0f),
            glm::vec3(-245.0f, 10.0f,  75.0f)
        };

        const glm::vec3 spotDirections[4] = {
            glm::normalize(glm::vec3(-0.7f, -0.65f,  0.15f)),
            glm::normalize(glm::vec3(-0.7f, -0.65f, -0.15f)),
            glm::normalize(glm::vec3( 0.8f, -0.55f,  0.2f)),
            glm::normalize(glm::vec3( 0.8f, -0.55f, -0.2f))
        };

        for (int i = 0; i < 4; ++i) {
            std::string n = std::to_string(i);
            phongShader.setVec3("spotLights[" + n + "].position", spotPositions[i]);
            phongShader.setVec3("spotLights[" + n + "].direction", spotDirections[i]);
            phongShader.setVec3("spotLights[" + n + "].ambient", 0.03f, 0.03f, 0.03f);
            phongShader.setVec3("spotLights[" + n + "].diffuse", 1.15f, 1.15f, 1.10f);
            phongShader.setVec3("spotLights[" + n + "].specular", 1.2f, 1.2f, 1.15f);
            phongShader.setFloat("spotLights[" + n + "].cutOff", glm::cos(glm::radians(19.0f)));
            phongShader.setFloat("spotLights[" + n + "].constant", 1.0f);
            phongShader.setFloat("spotLights[" + n + "].linear", 0.024f);
            phongShader.setFloat("spotLights[" + n + "].quadratic", 0.0018f);
        }

        // MVP + render
        int fbWidth = SCR_WIDTH;
        int fbHeight = SCR_HEIGHT;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        if (!quadViewMode) {
            glViewport(0, 0, fbWidth, fbHeight);
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                (float)fbWidth / (float)fbHeight, 0.1f, 500.0f);
            glm::mat4 view = camera.GetViewMatrix();
            phongShader.setMat4("projection", projection);
            phongShader.setMat4("view", view);
            phongShader.setVec3("viewPos", camera.Position);
            scene.render(phongShader);
        } else {
            const int halfW = fbWidth / 2;
            const int halfH = fbHeight / 2;

            const glm::vec3 center(-40.0f, 0.0f, 5.0f);
            const float halfSceneX = 190.0f;
            const float halfSceneZ = 120.0f;

            // 1) Top View (Top Left) - Orthographic (closer framing)
            glViewport(0, halfH, halfW, halfH);
            glm::mat4 projection = glm::ortho(-halfSceneX, halfSceneX, -halfSceneZ, halfSceneZ, 0.1f, 900.0f);
            glm::mat4 view = glm::lookAt(glm::vec3(center.x, 220.0f, center.z), center, glm::vec3(0.0f, 0.0f, -1.0f));
            phongShader.setMat4("projection", projection);
            phongShader.setMat4("view", view);
            phongShader.setVec3("viewPos", glm::vec3(center.x, 220.0f, center.z));
            scene.render(phongShader);

            // 2) Isometric View (Top Right) - Orthographic (closer framing)
            glViewport(halfW, halfH, halfW, halfH);
            projection = glm::ortho(-halfSceneX, halfSceneX, -halfSceneZ, halfSceneZ, 0.1f, 1200.0f);
            view = glm::lookAt(center + glm::vec3(185.0f, 175.0f, 185.0f), center + glm::vec3(0.0f, 16.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            phongShader.setMat4("projection", projection);
            phongShader.setMat4("view", view);
            phongShader.setVec3("viewPos", center + glm::vec3(185.0f, 175.0f, 185.0f));
            scene.render(phongShader);

            // 3) Front View (Bottom Left) - Orthographic (from gate side)
            glViewport(0, 0, halfW, halfH);
            projection = glm::ortho(-halfSceneZ, halfSceneZ, -20.0f, 95.0f, 0.1f, 1400.0f);
            view = glm::lookAt(center + glm::vec3(260.0f, 22.0f, 0.0f), center + glm::vec3(0.0f, 22.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            phongShader.setMat4("projection", projection);
            phongShader.setMat4("view", view);
            phongShader.setVec3("viewPos", center + glm::vec3(260.0f, 22.0f, 0.0f));
            scene.render(phongShader);

            // 4) Inside View (Bottom Right) - Perspective, Interactive
            glViewport(halfW, 0, halfW, halfH);
            projection = glm::perspective(glm::radians(camera.Zoom),
                (float)halfW / (float)halfH, 0.1f, 500.0f);
            view = camera.GetViewMatrix();
            phongShader.setMat4("projection", projection);
            phongShader.setMat4("view", view);
            phongShader.setVec3("viewPos", camera.Position);
            scene.render(phongShader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Movement (with sprint)
    float speedMult = 1.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        speedMult = 3.0f;

    float originalSpeed = camera.MovementSpeed;
    camera.MovementSpeed *= speedMult;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    camera.MovementSpeed = originalSpeed;

    // Arrow key look
    float rotSpeed = 80.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.ProcessKeyboardRotation(-rotSpeed, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.ProcessKeyboardRotation(rotSpeed, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.ProcessKeyboardRotation(0.0f, rotSpeed);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.ProcessKeyboardRotation(0.0f, -rotSpeed);

    // Wireframe toggle
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !keyFPressed) {
        wireframeMode = !wireframeMode;
        keyFPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
        keyFPressed = false;

    // Lighting toggles required by assignment
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !key1Pressed) {
        enableDirLight = !enableDirLight;
        key1Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) key1Pressed = false;

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !key2Pressed) {
        enablePointLight = !enablePointLight;
        key2Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) key2Pressed = false;

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !key3Pressed) {
        enableSpotLight = !enableSpotLight;
        key3Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) key3Pressed = false;

    // Texture mode interaction button: cycle without/with/blended
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !key4Pressed) {
        textureDisplayMode = (textureDisplayMode + 1) % 3;
        key4Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE) key4Pressed = false;

    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && !key5Pressed) {
        enableAmbient = !enableAmbient;
        key5Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE) key5Pressed = false;

    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS && !key6Pressed) {
        enableDiffuse = !enableDiffuse;
        key6Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE) key6Pressed = false;

    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS && !key7Pressed) {
        enableSpecular = !enableSpecular;
        key7Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE) key7Pressed = false;

    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS && !key8Pressed) {
        barrackDoorsOpen = !barrackDoorsOpen;
        key8Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE) key8Pressed = false;

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !keyVPressed) {
        quadViewMode = !quadViewMode;
        keyVPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE) keyVPressed = false;

    // Mouse interaction toggle
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !keyMPressed) {
        mouseCaptured = !mouseCaptured;
        glfwSetInputMode(window, GLFW_CURSOR, mouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        firstMouse = true;
        keyMPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) keyMPressed = false;

    // Optional: left click captures mouse once (does not toggle repeatedly)
    bool leftNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (leftNow && !leftMousePressed && !mouseCaptured) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        mouseCaptured = true;
        firstMouse = true;
    }
    leftMousePressed = leftNow;

    // Boundary clamping
    camera.Position.x = glm::clamp(camera.Position.x, CAMERA_MIN_X, CAMERA_MAX_X);
    camera.Position.z = glm::clamp(camera.Position.z, CAMERA_MIN_Z, CAMERA_MAX_Z);
    camera.Position.y = glm::clamp(camera.Position.y, 1.0f, 60.0f);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!mouseCaptured) return;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
