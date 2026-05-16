#pragma once
#include <stdio.h>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// -------- Estado global para conmutar la lámpara  --------
struct LampState {
    int   pointIdx = -1;      // índice de la point light a controlar
    bool  on = true;          // true = encendida
    glm::vec3 bulbPos{ 0.0f };  // posición de la bombilla
};

class Window
{
public:
    Window();
    Window(GLint windowWidth, GLint windowHeight);
    int Initialise();

    GLfloat getBufferWidth() { return bufferWidth; }
    GLfloat getBufferHeight() { return bufferHeight; }
    GLfloat getXChange();
    GLfloat getYChange();
    GLfloat getmuevex() { return muevex; }
    bool    getShouldClose() { return glfwWindowShouldClose(mainWindow); }
    bool* getsKeys() { return keys; }
    void    swapBuffers() { glfwSwapBuffers(mainWindow); }

    // límites ya usados
    float getPataFL() const { float v = articulacion1; return v < -45.0f ? -45.0f : (v > 45.0f ? 45.0f : v); }
    float getPataFR() const { float v = articulacion2; return v < -45.0f ? -45.0f : (v > 45.0f ? 45.0f : v); }
    float getPataRL() const { float v = articulacion3; return v < -45.0f ? -45.0f : (v > 45.0f ? 45.0f : v); }
    float getPataRR() const { float v = articulacion4; return v < -45.0f ? -45.0f : (v > 45.0f ? 45.0f : v); }
    float getJaw()    const { float v = articulacion5; return v < 0.0f ? 0.0f : (v > 35.0f ? 35.0f : v); }

    void adjustPataFL(float d) { articulacion1 = glm::clamp(articulacion1 + d, -45.0f, 45.0f); }
    void adjustPataFR(float d) { articulacion2 = glm::clamp(articulacion2 + d, -45.0f, 45.0f); }
    void adjustPataRL(float d) { articulacion3 = glm::clamp(articulacion3 + d, -45.0f, 45.0f); }
    void adjustPataRR(float d) { articulacion4 = glm::clamp(articulacion4 + d, -45.0f, 45.0f); }
    void adjustJaw(float d) { articulacion5 = glm::clamp(articulacion5 + d, 0.0f, 35.0f); }

    // --- Carro (si lo usas desde aquí) ---
    float carWheelAng;
    float carHoodAng;
    float carPosZ;
    float carYaw;

    // === Helpers de control ===
    template<typename CarT>
    void ProcessGameplayInput(CarT& car, float deltaTime) {
        const bool* k = this->getsKeys();
        const float hoodSpeed = 25.0f; // grados/seg
        if (k[GLFW_KEY_T]) car.openHood(+hoodSpeed * deltaTime);
        if (k[GLFW_KEY_G]) car.openHood(-hoodSpeed * deltaTime);
    }

    template<typename OffT>
    void ProcessDebugTuning(OffT& off) {
        const bool* k = this->getsKeys();
        const float nud = 0.1f;
        // HoodLocal
        if (k[GLFW_KEY_1]) off.hoodLocal.x += nud;
        if (k[GLFW_KEY_2]) off.hoodLocal.x -= nud;
        if (k[GLFW_KEY_3]) off.hoodLocal.y += nud;
        if (k[GLFW_KEY_4]) off.hoodLocal.y -= nud;
        if (k[GLFW_KEY_5]) off.hoodLocal.z += nud;
        if (k[GLFW_KEY_6]) off.hoodLocal.z -= nud;
        // HoodPivot
        if (k[GLFW_KEY_7]) off.hoodPivot.x += nud;
        if (k[GLFW_KEY_8]) off.hoodPivot.x -= nud;
        if (k[GLFW_KEY_9]) off.hoodPivot.y += nud;
        if (k[GLFW_KEY_0]) off.hoodPivot.y -= nud;
        if (k[GLFW_KEY_MINUS]) off.hoodPivot.z += nud;
        if (k[GLFW_KEY_EQUAL]) off.hoodPivot.z -= nud;
    }

    template<typename HeliT>
    void ProcessHelicopterInput(HeliT& heli, float deltaTime) {
        const bool* k = this->getsKeys();
        const float d = heli.speed * deltaTime;
        const float r = glm::radians(heli.yawDeg);
        const glm::vec3 fwd = glm::normalize(glm::vec3(-cosf(r), 0.0f, sinf(r)));
        if (k[GLFW_KEY_UP])    heli.pos += fwd * d;
        if (k[GLFW_KEY_DOWN])  heli.pos -= fwd * d;
    }

    // Toggle de lámpara (sin leer teclas en main)
    void ProcessLampToggle(LampState& lamp, float dt);

    template<typename HeliT, typename SpotLightT>
    void UpdateHelicopterSpotlight(const HeliT& heli, SpotLightT& sl) {
        const glm::vec3 spotPos = heli.pos + glm::vec3(0.0f, -0.5f, 0.0f);
        const float r = glm::radians(heli.yawDeg);
        const glm::vec3 fwd = glm::normalize(glm::vec3(-cosf(r), 0.0f, sinf(r)));
        const glm::vec3 spotDir = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f) * 0.85f + fwd * 0.35f);
        sl.SetFlash(spotPos, spotDir);
    }

    ~Window();

private:
    GLFWwindow* mainWindow = nullptr;
    GLint   width = 0, height = 0;
    GLfloat rotax = 0, rotay = 0, rotaz = 0;
    GLfloat articulacion1 = 0, articulacion2 = 0, articulacion3 = 0, articulacion4 = 0, articulacion5 = 0, articulacion6 = 0;
    bool    keys[1024]{};
    GLint   bufferWidth = 0, bufferHeight = 0;
    void    createCallbacks();
    GLfloat lastX = 0, lastY = 0;
    GLfloat xChange = 0, yChange = 0;
    GLfloat muevex = 0;
    bool    mouseFirstMoved = true;

    static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
    static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};
