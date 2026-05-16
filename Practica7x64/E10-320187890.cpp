#define STB_IMAGE_IMPLEMENTATION
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "Material.h"
#include "modelAnim.h" 

Window mainWindow;
std::vector<Shader*> shaderList;
Camera camera;
Model Auditorio_M;
Skybox skybox;
Material materialBrillante;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
DirectionalLight mainLight;
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";
static const char* vAnimShader = "shaders/anim.vert";
static const char* fAnimShader = "shaders/anim.frag";

int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();
    glEnable(GL_DEPTH_TEST);

    // Shader original (Estático)
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(shader1);

    // Cargar shader de animación
    Shader* animShader = new Shader();
    animShader->CreateFromFiles(vAnimShader, fAnimShader);
    shaderList.push_back(animShader);
    GLuint animProgramID = animShader->GetProgramID();

    // Cámara inicializada en Z = 50.0f
    camera = Camera(glm::vec3(0.0f, 5.0f, 50.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 10.0f, 0.5f);

    Auditorio_M = Model();
    Auditorio_M.LoadModel("Models/auditorio/auditorio.obj");

    // Cargar el modelo animado de caminata
    ModelAnim animacionPersonaje("Models/Animaciones/Walking.dae");
    animacionPersonaje.initShaders(animProgramID);

    materialBrillante = Material(1.0f, 32.0f);

    std::vector<std::string> skyboxFaces = {
        "Textures/Skybox/cupertin-lake_rt.tga", "Textures/Skybox/cupertin-lake_lf.tga",
        "Textures/Skybox/cupertin-lake_dn.tga", "Textures/Skybox/cupertin-lake_up.tga",
        "Textures/Skybox/cupertin-lake_bk.tga", "Textures/Skybox/cupertin-lake_ft.tga"
    };
    skybox = Skybox(skyboxFaces);

    mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 0.3f, 0.8f, 0.0f, -1.0f, -1.0f);

    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 2000.0f);

    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

        // =======================================================
        // 1. RENDERIZADO DEL ESCENARIO ESTÁTICO (Auditorio)
        // =======================================================
        shaderList[0]->UseShader();

        GLuint uniformModel = shaderList[0]->GetModelLocation();
        GLuint uniformProjection = shaderList[0]->GetProjectionLocation();
        GLuint uniformView = shaderList[0]->GetViewLocation();
        GLuint uniformEyePosition = shaderList[0]->GetEyePositionLocation();
        GLuint uniformSpecularIntensity = shaderList[0]->GetSpecularIntensityLocation();
        GLuint uniformShininess = shaderList[0]->GetShininessLocation();
        GLuint uniformColor = shaderList[0]->getColorLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

        shaderList[0]->SetDirectionalLight(&mainLight);
        materialBrillante.UseMaterial(uniformSpecularIntensity, uniformShininess);

        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(uniformColor, 1.0f, 1.0f, 1.0f);

        glActiveTexture(GL_TEXTURE0);
        Auditorio_M.RenderModel();


        // =======================================================
        // 2. RENDERIZADO DEL MODELO ANIMADO (Caminata Lineal)
        // =======================================================
        shaderList[1]->UseShader();

        GLuint modelLoc = glGetUniformLocation(animProgramID, "model");
        GLuint viewLoc = glGetUniformLocation(animProgramID, "view");
        GLuint projLoc = glGetUniformLocation(animProgramID, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glUniform3f(glGetUniformLocation(animProgramID, "material.specular"), 0.5f, 0.5f, 0.5f);
        glUniform1f(glGetUniformLocation(animProgramID, "material.shininess"), 32.0f);
        glUniform3f(glGetUniformLocation(animProgramID, "light.ambient"), 0.3f, 0.3f, 0.3f);
        glUniform3f(glGetUniformLocation(animProgramID, "light.diffuse"), 0.8f, 0.8f, 0.8f);
        glUniform3f(glGetUniformLocation(animProgramID, "light.specular"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(animProgramID, "light.direction"), 0.0f, -1.0f, -1.0f);

        // --- SECCIÓN COORDENADAS: ZONA DE LOS STANDS Y ASIENTOS ---

        // 1. Posición central del pasillo de los stands
        float centroX = 0.0f;       // Centrado en el pasillo entre bloques de stands
        float centroY = -3.2f;      // Altura del suelo del auditorio
        float centroZ = -30.0f;      // Ajustado a  para traerlo al frente donde están los stands

        // 2. Límites de caminata
        float largoPasillo = 3.5f;      // Qué tanto camina a los lados antes de dar la vuelta
        float velocidadCaminata = 0.4f;  // Velocidad del avance lineal

        // Simulación matemática del vaivén de izquierda a derecha
        float tiempoAnim = glfwGetTime() * velocidadCaminata;
        float posX = centroX + (sin(tiempoAnim) * largoPasillo);
        float posZ = centroZ;

        // 3. Rotación en Y para cambiar la vista al dar la vuelta (90° o -90°)
        float rotacionY = (cos(tiempoAnim) > 0.0f) ? 90.0f : -90.0f;

        // Matrices de transformación para el personaje
        glm::mat4 modelAnimMat = glm::mat4(1.0f);
        modelAnimMat = glm::translate(modelAnimMat, glm::vec3(posX, centroY, posZ));
        modelAnimMat = glm::rotate(modelAnimMat, glm::radians(rotacionY), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAnimMat = glm::scale(modelAnimMat, glm::vec3(0.02f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelAnimMat));

        // Dibujar el modelo con animación por huesos activa
        animacionPersonaje.Draw(animProgramID);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}