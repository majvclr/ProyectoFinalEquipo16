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

Window mainWindow;
std::vector<Shader*> shaderList; // Mantenemos punteros para estabilidad
Camera camera;
Model Auditorio_M;
Skybox skybox;

// Definición de materiales para dar profundidad
Material materialBrillante;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

DirectionalLight mainLight;

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    glEnable(GL_DEPTH_TEST);

    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(shader1);

    // 1. AJUSTE DE CÁMARA (Vista Interior similar a Blender)
    // Nos metemos al auditorio (Z=50) y subimos un poco (Y=5)
    camera = Camera(glm::vec3(0.0f, 5.0f, 50.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 10.0f, 0.5f);

    // Carga del modelo (Mantenemos la ruta)
    Auditorio_M = Model();
    Auditorio_M.LoadModel("Models/auditorio/auditorio.obj");

    // 2. ACTIVACIÓN DE MATERIAL (Adiós efecto cartón)
    // Inicializamos un material con brillo (Specular Intensity, Shininess)
    // El brillo (Shininess) en 32 es un estándar para materiales lisos.
    materialBrillante = Material(1.0f, 32.0f);

    std::vector<std::string> skyboxFaces = {
        "Textures/Skybox/cupertin-lake_rt.tga", "Textures/Skybox/cupertin-lake_lf.tga",
        "Textures/Skybox/cupertin-lake_dn.tga", "Textures/Skybox/cupertin-lake_up.tga",
        "Textures/Skybox/cupertin-lake_bk.tga", "Textures/Skybox/cupertin-lake_ft.tga"
    };
    skybox = Skybox(skyboxFaces);

    // 3. CAMBIO CRÍTICO EN LA LUZ (Contraste y Volumen)
    // Bajamos el ambiental (0.3f) y subimos el difuso (0.8f).
    // Antes el ambiental era alto, lo que "aplastaba" el volumen. 
    // Ahora la luz difusa (el Sol) creará sombras propias que definen las aristas.
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

        // 4. USO DEL MATERIAL (Paso final para la profundidad)
        // Le dice al Fragment Shader que este objeto tiene brillo especular.
        materialBrillante.UseMaterial(uniformSpecularIntensity, uniformShininess);

        glm::mat4 model(1.0f);
        // Pequeño ajuste para no aparecer enterrados
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(uniformColor, 1.0f, 1.0f, 1.0f);

        // Activamos unidad de textura 0 por seguridad
        glActiveTexture(GL_TEXTURE0);

        Auditorio_M.RenderModel();

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}