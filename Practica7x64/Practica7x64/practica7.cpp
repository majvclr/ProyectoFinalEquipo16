/*
Práctica 7: Iluminación 1
*/
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <array>
#include <math.h>

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
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;

// --- D8 ---
Texture texD8;
Mesh* meshD8 = nullptr;

Model Kitt_M;
Model Llanta_M;
Model Blackhawk_M;
Model Goddard_M;
Model StreetLamp_M;
Model Lampara_M;
Model ParedLadrillo_M;
Model Crustaceo_M;
Skybox skybox;

// Escala/posiciones fijas
const float kLampScale = 2.0f;
const glm::vec3 streetLampPos = glm::vec3(6.0f, -1.3f, -40.0f);

// Lámpara pequeña (no la StreetLamp)
glm::vec3 gLampWorldPos = glm::vec3(0.0f, -2.2f, -30.0f);
float     gLampScale = 45.0f;

// Materiales
Material Material_brillante;
Material Material_opaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// luces
DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight  spotLights[MAX_SPOT_LIGHTS];

// Shaders
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

/*===============Carro=======================*/

struct CarOffsetsRaw {
    glm::vec3 wheelFL{ 0 }, wheelFR{ 0 }, wheelRL{ 0 }, wheelRR{ 0 };
    glm::vec3 wheelCenter{ 0 };
    glm::vec3 hoodPivot{ 0 };   // pivote en ESPACIO DEL CUERPO
    glm::vec3 hoodLocal{ 0 };   // origen local del cofre (en el cuerpo)
    glm::vec3 headLLocal{ 0 };
    glm::vec3 headRLocal{ 0 };
    glm::vec3 wsLocal{ 0 };
    float     wsYawDeg{ 0.0f };
};

struct Car {
    Model body, hood, wheel, windshield;

    float scale = 7.4f;
    glm::vec3 pos = glm::vec3(0);
    float yawDeg = 0.0f;
    float wheelAng = 0.0f;
    float hoodAng = 0.0f;
    float hoodMax = 50.0f;

    CarOffsetsRaw raw{};
    struct {
        glm::vec3 FL{ 0 }, FR{ 0 }, RL{ 0 }, RR{ 0 }, Ctr{ 0 };
        glm::vec3 HoodPivot{ 0 }, HoodLocal{ 0 };
        glm::vec3 WsLocal{ 0 };
        glm::vec3 HeadL{ 0 }, HeadR{ 0 };
    } comp;
    float wsYawDeg = 180.0f;

    void setScale(float s) { scale = s; recomputeComp(); }
    void setBasePosition(const glm::vec3& p) { pos = p; }
    void setOffsetsRaw(const CarOffsetsRaw& o) { raw = o; recomputeComp(); }
    void setHoodMax(float deg) { hoodMax = deg; }

    void moveWorldZ(float dz) { pos.z += dz; }
    void addYaw(float ddeg) { yawDeg += ddeg; }
    void moveLocalForward(float d) {
        float r = glm::radians(yawDeg);
        pos += glm::vec3(-sinf(r), 0.0f, -cosf(r)) * d;
    }
    void spinWheels(float ddeg) { wheelAng += ddeg; }
    void openHood(float ddeg) { hoodAng = glm::clamp(hoodAng + ddeg, 0.0f, hoodMax); }

    void getHeadlightWorld(bool left, glm::vec3& outPos, glm::vec3& outDir) const {
        // Solo traslación + rotación del coche (SIN aplicar scale a la posición del faro)
        glm::mat4 B(1.0f);
        B = glm::translate(B, pos);
        B = glm::rotate(B, glm::radians(yawDeg), glm::vec3(0, 1, 0));

        glm::vec3 loc = left ? comp.HeadL : comp.HeadR;
        outPos = glm::vec3(B * glm::vec4(loc, 1.0f));   // <-- sin escala

        // Dirección del faro: depende solo del yaw (tu “frente” es +Z)
        glm::mat4 R(1.0f);
        R = glm::rotate(R, glm::radians(yawDeg), glm::vec3(0, 1, 0));
        outDir = glm::vec3(R * glm::vec4(0, 0, +1, 0));
    }


    void render(GLint uModel, GLint uColor) {
        glm::mat4 B(1.0f);
        B = glm::translate(B, pos);
        B = glm::rotate(B, glm::radians(yawDeg), glm::vec3(0, 1, 0));

        // Carrocería
        glm::mat4 M = glm::scale(B, glm::vec3(scale));
        glUniform3fv(uColor, 1, glm::value_ptr(glm::vec3(1.0f)));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
        body.RenderModel();

        // Cofre
        glm::mat4 H = B;
        H = glm::translate(H, comp.HoodLocal);
        H = glm::translate(H, comp.HoodPivot);
        H = glm::rotate(H, glm::radians(-hoodAng), glm::vec3(1, 0, 0));
        H = glm::translate(H, -comp.HoodPivot);
        H = glm::scale(H, glm::vec3(scale));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(H));
        hood.RenderModel();

        // Parabrisas
        glm::mat4 W = B;
        W = glm::translate(W, comp.WsLocal);
        W = glm::rotate(W, glm::radians(wsYawDeg), glm::vec3(0, 1, 0));
        W = glm::scale(W, glm::vec3(scale));
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(W));
        windshield.RenderModel();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        // Llantas
        auto drawWheel = [&](const glm::vec3& p) {
            glm::mat4 Wm = B;
            Wm = glm::translate(Wm, p);
            Wm = glm::translate(Wm, comp.Ctr);
            Wm = glm::rotate(Wm, glm::radians(wheelAng), glm::vec3(1, 0, 0));
            Wm = glm::translate(Wm, -comp.Ctr);
            Wm = glm::scale(Wm, glm::vec3(scale));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(Wm));
            wheel.RenderModel();
            };
        drawWheel(comp.FL); drawWheel(comp.FR); drawWheel(comp.RL); drawWheel(comp.RR);
    }

private:
    void recomputeComp() {
        comp.FL = raw.wheelFL;  comp.FR = raw.wheelFR;
        comp.RL = raw.wheelRL;  comp.RR = raw.wheelRR;
        comp.Ctr = raw.wheelCenter;
        comp.HoodLocal = raw.hoodLocal;
        comp.HoodPivot = raw.hoodPivot - raw.hoodLocal; // pivote en espacio LOCAL del cofre
        comp.WsLocal = raw.wsLocal;
        wsYawDeg = raw.wsYawDeg;
        comp.HeadL = raw.headLLocal;
        comp.HeadR = raw.headRLocal;
    }
};

// normales
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount,
    GLfloat* vertices, unsigned int verticeCount, unsigned int vLength, unsigned int normalOffset)
{
    for (size_t i = 0; i < indiceCount; i += 3)
    {
        unsigned int in0 = indices[i] * vLength;
        unsigned int in1 = indices[i + 1] * vLength;
        unsigned int in2 = indices[i + 2] * vLength;
        glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
        glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
        glm::vec3 normal = glm::normalize(glm::cross(v1, v2));
        in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
        vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
        vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
        vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
    }
    for (size_t i = 0; i < verticeCount / vLength; i++)
    {
        unsigned int nOffset = i * vLength + normalOffset;
        glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
        vec = glm::normalize(vec);
        vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
    }
}

void CreateObjects()
{
    unsigned int indices[] = { 0,3,1, 1,3,2, 2,3,0, 0,1,2 };

    GLfloat vertices[] = {
        //  x      y      z         u     v         nx    ny    nz
        -1.0f, -1.0f, -0.6f,    0.0f, 0.0f,      0.0f, 0.0f, 0.0f,
         0.0f, -1.0f,  1.0f,    0.5f, 0.0f,      0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, -0.6f,    1.0f, 0.0f,      0.0f, 0.0f, 0.0f,
         0.0f,  1.0f,  0.0f,    0.5f, 1.0f,      0.0f, 0.0f, 0.0f
    };

    unsigned int floorIndices[] = { 0,2,1, 1,2,3 };
    GLfloat floorVertices[] = {
        -10.0f, 0.0f, -10.0f,   0.0f, 0.0f,      0.0f, -1.0f, 0.0f,
         10.0f, 0.0f, -10.0f,  10.0f, 0.0f,      0.0f, -1.0f, 0.0f,
        -10.0f, 0.0f,  10.0f,   0.0f, 10.0f,     0.0f, -1.0f, 0.0f,
         10.0f, 0.0f,  10.0f,  10.0f, 10.0f,     0.0f, -1.0f, 0.0f
    };

    unsigned int vegetacionIndices[] = { 0,1,2, 0,2,3, 4,5,6, 4,6,7 };
    GLfloat vegetacionVertices[] = {
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f,      0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,     1.0f, 0.0f,      0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, 0.0f,     1.0f, 1.0f,      0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,     0.0f, 1.0f,      0.0f, 0.0f, 0.0f,

         0.0f, -0.5f, -0.5f,    0.0f, 0.0f,      0.0f, 0.0f, 0.0f,
         0.0f, -0.5f,  0.5f,    1.0f, 0.0f,      0.0f, 0.0f, 0.0f,
         0.0f,  0.5f,  0.5f,    1.0f, 1.0f,      0.0f, 0.0f, 0.0f,
         0.0f,  0.5f, -0.5f,    0.0f, 1.0f,      0.0f, 0.0f, 0.0f,
    };

    Mesh* obj1 = new Mesh(); obj1->CreateMesh(vertices, indices, 32, 12); meshList.push_back(obj1);
    Mesh* obj2 = new Mesh(); obj2->CreateMesh(vertices, indices, 32, 12); meshList.push_back(obj2);
    Mesh* obj3 = new Mesh(); obj3->CreateMesh(floorVertices, floorIndices, 32, 6); meshList.push_back(obj3);
    Mesh* obj4 = new Mesh(); obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12); meshList.push_back(obj4);

    calcAverageNormals(indices, 12, vertices, 32, 8, 5);
    calcAverageNormals(vegetacionIndices, 12, vegetacionVertices, 64, 8, 5);
}

void CreateD8()
{
    unsigned int cubo_indices[] = {
        0,1,2,  3,4,5,  6,7,8,  9,10,11,
        12,13,14, 14,15,12,
        16,17,18, 19,20,21, 22,23,24, 25,26,27,
    };
    auto UVtop = [](float cx, float cy) {
        const float du = 0.065f;
        const float dv = 0.085f;
        return std::array<glm::vec2, 3>{
            glm::vec2(cx - du, cy - dv),
                glm::vec2(cx + du, cy - dv),
                glm::vec2(cx, cy + dv)
        };
        };
    auto UVbot = [](float cx, float cy) {
        const float du = 0.065f;
        const float dv = 0.085f;
        return std::array<glm::vec2, 3>{
            glm::vec2(cx - du, cy + dv),
                glm::vec2(cx + du, cy + dv),
                glm::vec2(cx, cy - dv)
        };
        };

    const glm::vec2 C1 = { 0.50f, 0.43f };
    const glm::vec2 C2 = { 0.80f, 0.60f };
    const glm::vec2 C3 = { 0.72f, 0.30f };
    const glm::vec2 C4 = { 0.50f, 0.60f };
    const glm::vec2 C5 = { 0.50f, 0.16f };
    const glm::vec2 C6 = { 0.20f, 0.60f };
    const glm::vec2 C7 = { 0.28f, 0.30f };
    const glm::vec2 C8 = { 0.50f, 0.84f };

    auto uv1 = UVtop(C1.x, C1.y);
    auto uv2 = UVtop(C2.x, C2.y);
    auto uv3 = UVtop(C3.x, C3.y);
    auto uv4 = UVtop(C4.x, C4.y);
    auto uv5 = UVbot(C5.x, C5.y);
    auto uv6 = UVbot(C6.x, C6.y);
    auto uv7 = UVbot(C7.x, C7.y);
    auto uv8 = UVbot(C8.x, C8.y);

    GLfloat cubo_vertices[] = {
        // -------- TOP
        -0.5f,-0.5f, 0.5f, uv1[0].x, uv1[0].y,  0, 1,-1,
         0.5f,-0.5f, 0.5f, uv1[1].x, uv1[1].y,  0, 1,-1,
         0.0f, 0.5f, 0.0f, uv1[2].x, uv1[2].y,  0, 1,-1,

         0.5f,-0.5f, 0.5f, uv2[0].x, uv2[0].y, -1, 1, 0,
         0.5f,-0.5f,-0.5f, uv2[1].x, uv2[1].y, -1, 1, 0,
         0.0f, 0.5f, 0.0f, uv2[2].x, uv2[2].y, -1, 1, 0,

         0.5f,-0.5f,-0.5f, uv3[0].x, uv3[0].y,  0, 1, 1,
        -0.5f,-0.5f,-0.5f, uv3[1].x, uv3[1].y,  0, 1, 1,
         0.0f, 0.5f, 0.0f, uv3[2].x, uv3[2].y,  0, 1, 1,

        -0.5f,-0.5f,-0.5f, uv4[0].x, uv4[0].y,  1, 1, 0,
        -0.5f,-0.5f, 0.5f, uv4[1].x, uv4[1].y,  1, 1, 0,
         0.0f, 0.5f, 0.0f, uv4[2].x, uv4[2].y,  1, 1, 0,

         // base superior
         -0.5f,-0.5f, 0.5f, uv1[0].x, uv1[0].y, 0,1,0,
          0.5f,-0.5f, 0.5f, uv1[1].x, uv1[1].y, 0,1,0,
          0.5f,-0.5f,-0.5f, uv1[1].x, uv1[2].y, 0,1,0,
         -0.5f,-0.5f,-0.5f, uv1[0].x, uv1[2].y, 0,1,0,

         // -------- BOTTOM (ápice abajo)
         -0.5f,-0.5f, 0.5f, uv5[0].x, uv5[0].y,  0,-1,-1,
          0.5f,-0.5f, 0.5f, uv5[1].x, uv5[1].y,  0,-1,-1,
          0.0f,-1.5f, 0.0f, uv5[2].x, uv5[2].y,  0,-1,-1,

          0.5f,-0.5f, 0.5f, uv6[0].x, uv6[0].y, -1,-1, 0,
          0.5f,-0.5f,-0.5f, uv6[1].x, uv6[1].y, -1,-1, 0,
          0.0f,-1.5f, 0.0f, uv6[2].x, uv6[2].y, -1,-1, 0,

          0.5f,-0.5f,-0.5f, uv7[0].x, uv7[0].y,  0,-1, 1,
         -0.5f,-0.5f,-0.5f, uv7[1].x, uv7[1].y,  0,-1, 1,
          0.0f,-1.5f, 0.0f, uv7[2].x, uv7[2].y,  0,-1, 1,

         -0.5f,-0.5f,-0.5f, uv8[0].x, uv8[0].y,  1,-1, 0,
         -0.5f,-0.5f, 0.5f,  uv8[1].x, uv8[1].y,  1,-1, 0,
          0.0f,-1.5f, 0.0f,  uv8[2].x, uv8[2].y,  1,-1, 0,
    };

    if (!meshD8) meshD8 = new Mesh();
    meshD8->CreateMesh(cubo_vertices, cubo_indices, 224, 30);

    texD8 = Texture("Textures/Dado2.png");
    texD8.LoadTextureA();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void RenderLeg(Model& legModel, GLuint uniformModel, const glm::mat4& base,
    const glm::vec3& hipLocalPos, float swingDeg, bool mirrorX,
    const glm::vec3& legOriginOffset = glm::vec3(0.0f))
{
    glm::mat4 M = base;
    M = glm::translate(M, hipLocalPos);
    M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0, 1, 0));
    M = glm::rotate(M, glm::radians(swingDeg), glm::vec3(1.0f, 0.0f, 0.0f));
    if (mirrorX) M = glm::scale(M, glm::vec3(-1.0f, 1.0f, 1.0f));
    M = glm::translate(M, legOriginOffset);
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(M));
    legModel.RenderModel();
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    printf("[Debug] gLampWorldPos = (%.2f, %.2f, %.2f), gLampScale = %.2f\n",
        gLampWorldPos.x, gLampWorldPos.y, gLampWorldPos.z, gLampScale);

    CreateObjects();
    CreateShaders();
    CreateD8(); // dado 8 caras

    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f, 0.0f, 0.3f, 0.5f);

    // Modelos
    Goddard_M = Model();
    Model Cuerpo_M, Mandibula_M, PataFront_M, PataRear_M;

    Cuerpo_M.LoadModel("Models/cuerpo.obj");
    Mandibula_M.LoadModel("Models/mandibula.obj");
    PataFront_M.LoadModel("Models/pata_delantera.obj");
    PataRear_M.LoadModel("Models/pata_trasera.obj");

    brickTexture = Texture("Textures/brick.png");  brickTexture.LoadTextureA();
    dirtTexture = Texture("Textures/dirt.png");   dirtTexture.LoadTextureA();
    plainTexture = Texture("Textures/plain.png");  plainTexture.LoadTextureA();
    pisoTexture = Texture("Textures/piso.tga");   pisoTexture.LoadTextureA();
    AgaveTexture = Texture("Textures/Agave.tga");  AgaveTexture.LoadTextureA();

    Kitt_M = Model();      Kitt_M.LoadModel("Models/kitt_optimizado.obj");
    Llanta_M = Model();    Llanta_M.LoadModel("Models/llanta_optimizada.obj");
    Blackhawk_M = Model(); Blackhawk_M.LoadModel("Models/uh60.obj");

    Lampara_M.LoadModel("Models/Lampara.obj");
    StreetLamp_M.LoadModel("Models/StreetLamp.obj");
    ParedLadrillo_M.LoadModel("Models/paredladrillo.obj");
    Crustaceo_M.LoadModel("Models/crustaceo.obj");

    std::vector<std::string> skyboxFaces = {
        "Textures/Skybox/cupertin-lake_rt.tga",
        "Textures/Skybox/cupertin-lake_lf.tga",
        "Textures/Skybox/cupertin-lake_dn.tga",
        "Textures/Skybox/cupertin-lake_up.tga",
        "Textures/Skybox/cupertin-lake_bk.tga",
        "Textures/Skybox/cupertin-lake_ft.tga"
    };
    skybox = Skybox(skyboxFaces);

    Material_brillante = Material(4.0f, 256);
    Material_opaco = Material(0.3f, 4);

    mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 0.3f, 0.3f, 0.0f, 0.0f, -1.0f);

    unsigned int pointLightCount = 0;
    // --- (NUEVO) 3) PointLight PURPURA para Crustáceo ---
    const int CRUST_POINT = pointLightCount;
    pointLights[pointLightCount++] = PointLight(
        0.60f, 0.20f, 0.80f,   // morado
        0.10f, 2.2f,
        0.0f, 0.0f, 0.0f,      // se actualiza cada frame
        1.0f, 0.022f, 0.0019f
    );

    pointLights[pointLightCount++] = PointLight(1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -6.0f, 1.5f, 1.5f, 1.0f, 0.2f, 0.1f);

    // --- Spotlights (índices estables para no romper tus teclas/inputs) ---
    unsigned int spotLightCount = 0;

    // 0) Flash de cámara (blanco)
    const int camSpotIdx = spotLightCount;
    spotLights[spotLightCount++] = SpotLight(
        1.0f, 1.0f, 1.0f, 0.0f, 2.0f,
        0, 0, 0,
        0, -1, 0,
        1.0f, 0.09f, 0.032f,
        12.0f
    );

    // 1) Verde de prueba (tu luz existente)
    spotLights[spotLightCount++] = SpotLight(
        0.0f, 1.0f, 0.0f, 1.0f, 2.0f,
        5.0f, 10.0f, 0.0f, 0.0f, -5.0f, 0.0f,
        1.0f, 0.045f, 0.0072f, 15.0f
    );

    // === Carro ===
    Car car;
    car.body.LoadModel("Models/CuerpoCarro.obj");
    car.hood.LoadModel("Models/CofreTextura.obj");
    car.wheel.LoadModel("Models/Llanta.obj");
    car.windshield.LoadModel("Models/Parabrisas.obj");
    car.setScale(7.4f);
    car.setBasePosition(glm::vec3(0.0f, -2.2f, -14.0f));

    CarOffsetsRaw off;
    off.wheelFL = glm::vec3(+47.7f, 0.0f, +13.35f);
    off.wheelFR = glm::vec3(+36.0f, 0.0f, +13.35f);
    off.wheelRL = glm::vec3(+47.7f, 0.0f, -8.4f);
    off.wheelRR = glm::vec3(+36.0f, 0.0f, -8.4f);
    off.wheelCenter = glm::vec3(0.0f, 0.0f, -0.12f);
    off.hoodLocal = glm::vec3(2.70f, 0.20f, 4.55f);
    off.hoodPivot = off.hoodLocal + glm::vec3(0.0f, 5.0f, 9.20f);
    off.wsLocal = glm::vec3(3.0f, 0.0f, 4.8f);
    off.wsYawDeg = 180.0f;
    off.headLLocal = glm::vec3(+10.0f, +6.5f, +15.0f);
    off.headRLocal = glm::vec3(-2.0f, +6.5f, +15.0f);
    car.setOffsetsRaw(off);
    car.setHoodMax(50.0f);

    // 2) y 3) Faros del coche (índices fijos para que no choquen con tu lógica)
    const int HL_L = spotLightCount++;
    const int HL_R = spotLightCount++;
    spotLights[HL_L] = SpotLight(
        0.30f, 0.65f, 1.00f, 0.06f, 10.0f,
        0, 0, 0, 0, 0, +1,
        1.0f, 0.009f, 0.00032f, 24.0f
    );
    spotLights[HL_R] = SpotLight(
        0.30f, 0.65f, 1.00f, 0.06f, 10.0f,
        0, 0, 0, 0, 0, +1,
        1.0f, 0.009f, 0.00032f, 24.0f
    );


    // --- (NUEVO) 1) Spotlight del COFRE (VERDE) ---
    const int HOOD_SPOT = spotLightCount++;
    spotLights[HOOD_SPOT] = SpotLight(
        0.10f, 0.95f, 0.10f,   // verde
        0.05f, 2.2f,
        0, 0, 0,               // se actualiza cada frame
        0, 0, +1,              // se actualiza cada frame
        1.0f, 0.009f, 0.00032f, 20.0f
    );

    // --- (NUEVO) 2) Spotlights de MOVIMIENTO ---
    const int MOVE_FWD_SPOT = spotLightCount++;
    spotLights[MOVE_FWD_SPOT] = SpotLight(
        1.00f, 0.55f, 0.10f,   // naranja
        0.04f, 1.8f,
        0, 0, 0, 0, 0, +1,
        1.0f, 0.009f, 0.00032f, 18.0f
    );
    const int MOVE_BACK_SPOT = spotLightCount++;
    spotLights[MOVE_BACK_SPOT] = SpotLight(
        1.00f, 1.00f, 0.10f,   // amarillo
        0.04f, 1.6f,
        0, 0, 0, 0, 0, -1,
        1.0f, 0.009f, 0.00032f, 18.0f
    );
    // Helicóptero
    struct HelicopterState { glm::vec3 pos{ -8.0f,6.0f,-12.0f }; float yawDeg{ 0 }; float speed{ 4.0f }; int spotIdx{ -1 }; } heli;
    {
        const GLfloat edgeDeg = 30.0f;
        int idx = spotLightCount++;
        heli.spotIdx = idx;
        spotLights[idx] = SpotLight(1.0f, 0.95f, 0.60f, 0.15f, 4.0f,
            heli.pos.x, heli.pos.y, heli.pos.z,
            0.0f, -1.0f, 0.0f, 1.0f, 0.045f, 0.0072f, edgeDeg);
    }

    // === Matrices iniciales de modelos ===
    const glm::mat4 lamparaModelInit =
        glm::scale(glm::translate(glm::mat4(1.0f), gLampWorldPos), glm::vec3(gLampScale));
    const glm::mat4 streetLampModelInit =
        glm::scale(glm::translate(glm::mat4(1.0f), streetLampPos), glm::vec3(kLampScale));

    // === LUCES DE LÁMPARAS ===
    LampState lamp;
    struct StreetLampState { int pointIdx{ -1 }; } street;

    {   // Pequeña 
        const glm::vec3 lampBulbLocal(0.0f, 1.0f, 0.0f);
        glm::vec3 bulb = glm::vec3(lamparaModelInit * glm::vec4(lampBulbLocal, 1.0f));
        lamp.bulbPos = bulb;
        pointLights[pointLightCount] = PointLight(
            1.0f, 0.9f, 0.6f, 0.3f, 3.0f,
            bulb.x, bulb.y, bulb.z,
            1.0f, 0.02f, 0.001f
        );
        lamp.pointIdx = pointLightCount;
        pointLightCount++;
    }
    {   // StreetLamp grande 
        street.pointIdx = pointLightCount;
        pointLights[pointLightCount] = PointLight(
            1.0f, 1.0f, 1.0f, 0.12f, 2.4f,
            streetLampPos.x, streetLampPos.y + kLampScale * 13.9f, streetLampPos.z,
            1.0f, 0.022f, 0.0019f
        );
        pointLightCount++;
    }

    // Pared / Crustáceo
    const glm::vec3 wallPos = car.pos + glm::vec3(0.0f, 0.0f, +45.0f);
    const glm::mat4 paredModel = glm::scale(
        glm::translate(glm::mat4(1.0f), wallPos + glm::vec3(0.0f, -2.2f, 0.0f)),
        glm::vec3(5.0f)
    );

    const glm::vec3 crustPos = glm::vec3(100.0f, -2.2f, -18.0f);
    const glm::mat4 crustModel = glm::scale(
        glm::translate(glm::mat4(1.0f), crustPos),
        glm::vec3(10.8f)
    );

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
        uniformSpecularIntensity = 0, uniformShininess = 0, uniformColor = 0;

    glm::mat4 projection = glm::perspective(45.0f,
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

        shaderList[0].UseShader();
        uniformModel = shaderList[0].GetModelLocation();
        uniformProjection = shaderList[0].GetProjectionLocation();
        uniformView = shaderList[0].GetViewLocation();
        uniformEyePosition = shaderList[0].GetEyePositionLocation();
        uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
        uniformShininess = shaderList[0].GetShininessLocation();
        uniformColor = shaderList[0].getColorLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

        // === INPUTS ===
        // Linterna ligada a cámara
        glm::vec3 lowerLight = camera.getCameraPosition(); lowerLight.y -= 0.3f;
        spotLights[camSpotIdx].SetFlash(lowerLight, camera.getCameraDirection());

        
        mainWindow.ProcessDebugTuning(off);
        car.setOffsetsRaw(off);

        // Gameplay: coche / giro de ruedas / abrir cofre, etc.
        mainWindow.ProcessGameplayInput(car, deltaTime);

        // Faros del coche sincronizados
        {
            glm::vec3 pL, dL, pR, dR;
            car.setBasePosition(glm::vec3(0.0f, -2.2f, -14.0f - mainWindow.getmuevex())); // tu desplazamiento
            car.getHeadlightWorld(true, pL, dL);
            car.getHeadlightWorld(false, pR, dR);

            // Inclinación ligera hacia abajo para que iluminen el piso/escena
            const glm::vec3 down(0.0f, -0.25f, 0.0f);
            dL = glm::normalize(dL + down);
            dR = glm::normalize(dR + down);

            spotLights[HL_L].SetFlash(pL, dL);
            spotLights[HL_R].SetFlash(pR, dR);
        }

        // --- (NUEVO) Spotlight del COFRE ---
        {
            glm::mat4 B(1.0f);
            B = glm::translate(B, car.pos);
            B = glm::rotate(B, glm::radians(car.yawDeg), glm::vec3(0, 1, 0));

            glm::mat4 H = B;
            H = glm::translate(H, car.comp.HoodLocal);
            H = glm::translate(H, car.comp.HoodPivot);
            H = glm::rotate(H, glm::radians(-car.hoodAng), glm::vec3(1, 0, 0));
            H = glm::translate(H, -car.comp.HoodPivot);
            H = glm::scale(H, glm::vec3(car.scale));

            const glm::vec3 localOrigin(0.0f, 0.15f, +1.2f);
            const glm::vec3 localDir(0.0f, 0.0f, +1.0f);

            const glm::vec3 worldPos = glm::vec3(H * glm::vec4(localOrigin, 1.0f));
            const glm::vec3 worldDir = glm::normalize(glm::vec3(H * glm::vec4(localDir, 0.0f)));

            float on = 1.0f;
            spotLights[HOOD_SPOT] = SpotLight(
                0.10f, 0.95f, 0.10f,
                0.05f * on, 2.2f * on,
                worldPos.x, worldPos.y, worldPos.z,
                worldDir.x, worldDir.y, worldDir.z,
                1.0f, 0.009f, 0.00032f, 20.0f
            );
        }

        // --- (NUEVO) Spotlights de MOVIMIENTO (Y/U) ---
        {
            static float prevM = 0.0f;
            const float curM = mainWindow.getmuevex();
            const float dm = curM - prevM;
            prevM = curM;

            // Avanzas cuando muevex AUMENTA (z va a −Z). Retrocedes cuando muevex DISMINUYE.
            const float eps = 0.0005f;
            bool advancing = (dm > eps);
            bool reversing = (dm < -eps);

            const bool* k = mainWindow.getsKeys();
            advancing = advancing || k[GLFW_KEY_Y];
            reversing = reversing || k[GLFW_KEY_U];

            glm::vec3 pL, dL, pR, dR;
            car.getHeadlightWorld(true, pL, dL);
            car.getHeadlightWorld(false, pR, dR);

            const glm::vec3 frontPos = (pL + pR) * 0.5f;
            glm::vec3 baseDir = glm::normalize((dL + dR) * 0.5f);
            glm::vec3 frontDir = -baseDir;     // hacia delante
            glm::vec3 backDir = baseDir;
            const float backOffset = 6.0f;
            const glm::vec3 backPos = frontPos - frontDir * backOffset;
            
            // Inclinación ligera hacia abajo
            const glm::vec3 down(0.0f, -0.25f, 0.0f);
            frontDir = glm::normalize(frontDir + down);
            backDir = glm::normalize(backDir + down);

            if (advancing) {
                // Naranja al frente
                spotLights[MOVE_FWD_SPOT] = SpotLight(
                    1.00f, 0.55f, 0.10f, 0.05f, 1.8f,
                    frontPos.x, frontPos.y, frontPos.z,
                    frontDir.x, frontDir.y, frontDir.z,
                    1.0f, 0.009f, 0.00032f, 18.0f
                );
            }
            else {
                spotLights[MOVE_FWD_SPOT] = SpotLight(
                    1.00f, 0.55f, 0.10f, 0.0f, 0.0f,
                    frontPos.x, frontPos.y, frontPos.z,
                    frontDir.x, frontDir.y, frontDir.z,
                    1.0f, 0.009f, 0.00032f, 18.0f
                );
            }

            if (reversing) {
                // Amarillo hacia atrás
                spotLights[MOVE_BACK_SPOT] = SpotLight(
                    1.00f, 1.00f, 0.10f, 0.05f, 1.6f,
                    backPos.x, backPos.y, backPos.z,
                    backDir.x, backDir.y, backDir.z,
                    1.0f, 0.009f, 0.00032f, 18.0f
                );
            }
            else {
                spotLights[MOVE_BACK_SPOT] = SpotLight(
                    1.00f, 1.00f, 0.10f, 0.0f, 0.0f,
                    backPos.x, backPos.y, backPos.z,
                    backDir.x, backDir.y, backDir.z,
                    1.0f, 0.009f, 0.00032f, 18.0f
                );
            }
        }



        // --- (NUEVO) Luz puntual morada del Crustáceo ---
        {
            const bool* k = mainWindow.getsKeys();
            static bool crustOn = true;
            static bool prev = false;
            bool pressed = k[GLFW_KEY_P];
            if (pressed && !prev) { crustOn = !crustOn; }
            prev = pressed;

            glm::vec3 crustBulb = glm::vec3(crustModel * glm::vec4(0, 4.0f, 0, 1));

            if (crustOn) {
                pointLights[CRUST_POINT] = PointLight(
                    0.60f, 0.20f, 0.80f,
                    0.10f, 2.2f,
                    crustBulb.x, crustBulb.y, crustBulb.z,
                    1.0f, 0.022f, 0.0019f
                );
            }
            else {
                pointLights[CRUST_POINT] = PointLight(
                    0.60f, 0.20f, 0.80f,
                    0.0f, 0.0f,
                    crustBulb.x, crustBulb.y, crustBulb.z,
                    1.0f, 0.022f, 0.0019f
                );
            }
        }


        // Helicóptero (si tus teclas lo usan)
        mainWindow.ProcessHelicopterInput(heli, deltaTime);
        if (heli.spotIdx >= 0) mainWindow.UpdateHelicopterSpotlight(heli, spotLights[heli.spotIdx]);

        // Toggle lámpara pequeña
        mainWindow.ProcessLampToggle(lamp, deltaTime);
        if (lamp.pointIdx >= 0) {
            const int i = lamp.pointIdx;
            if (lamp.on) {
                pointLights[i] = PointLight(1.0f, 0.9f, 0.6f, 0.3f, 3.0f,
                    lamp.bulbPos.x, lamp.bulbPos.y, lamp.bulbPos.z,
                    1.0f, 0.02f, 0.001f);
            }
            else {
                pointLights[i] = PointLight(1.0f, 0.9f, 0.6f, 0.0f, 0.0f,
                    lamp.bulbPos.x, lamp.bulbPos.y, lamp.bulbPos.z,
                    1.0f, 0.02f, 0.001f);
            }
        }

        // StreetLamp grande
        const glm::mat4 streetLampModelR =
            glm::scale(glm::translate(glm::mat4(1.0f), streetLampPos), glm::vec3(kLampScale));
        const glm::vec3 streetBulbWorld = glm::vec3(streetLampModelR * glm::vec4(0.0f, 13.9f, 0.0f, 1.0f));
        if (street.pointIdx >= 0) {
            pointLights[street.pointIdx] = PointLight(
                1.0f, 1.0f, 1.0f, 0.12f, 2.4f,
                streetBulbWorld.x, streetBulbWorld.y, streetBulbWorld.z,
                1.0f, 0.022f, 0.0019f
            );
        }

        // Subir luces a shader
        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, pointLightCount);
        shaderList[0].SetSpotLights(spotLights, spotLightCount);

        glm::mat4 model(1.0f);
        glm::vec3 color(1.0f);

        // Piso
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        pisoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[2]->RenderMesh();

        // Carro 
        car.render(uniformModel, uniformColor);

        // Helicóptero (modelo)
        model = glm::mat4(1.0f);
        model = glm::translate(model, heli.pos);
        model = glm::rotate(model, glm::radians(heli.yawDeg), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.3f));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(1, 0, 0));
        model = glm::rotate(model, 90 * toRadians, glm::vec3(0, 0, 1));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Blackhawk_M.RenderModel();

        // Lámpara pequeña (modelo)
        const glm::mat4 lamparaModelR =
            glm::scale(glm::translate(glm::mat4(1.0f), gLampWorldPos), glm::vec3(gLampScale));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(lamparaModelR));
        Lampara_M.RenderModel();

        // StreetLamp (modelo)
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(streetLampModelR));
        StreetLamp_M.RenderModel();

        // Agave
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, -4.0f));
        model = glm::scale(model, glm::vec3(4.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        AgaveTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[3]->RenderMesh();
        glDisable(GL_BLEND);

        // Dado 8
        {
            glm::mat4 M = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 4.5f, -2.0f));
            M = glm::scale(M, glm::vec3(1.2f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(M));
            glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(1.0f)));
            texD8.UseTexture();
            Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
            meshD8->RenderMesh();
        }

        // Pared y Crustáceo
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(paredModel));
        ParedLadrillo_M.RenderModel();

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(crustModel));
        Crustaceo_M.RenderModel();

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}







