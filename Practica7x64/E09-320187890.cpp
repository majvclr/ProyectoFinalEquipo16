/*
Práctica 9
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

// Texturas base
Texture brickTexture, dirtTexture, plainTexture, pisoTexture, AgaveTexture;

// Texturas animadas / números
Texture FlechaTexture, NumerosTexture, Numero1Texture, Numero2Texture;

// Textura dragón
Texture PielDragonTexture;

// D8
Texture texD8;
Mesh* meshD8 = nullptr;

// Modelos
Model Kitt_M, Llanta_M, Blackhawk_M;
Model StreetLamp_M, Lampara_M, ParedLadrillo_M, Crustaceo_M;

// Dragón nuevo (cuerpo + alas)
Model CuerpoDragon_M, AlasDragon_M;

// Cielo
Skybox skybox;

// Lámparas
const float     kLampScale = 2.0f;
const glm::vec3 streetLampPos = glm::vec3(6.0f, -1.3f, -40.0f);
glm::vec3       gLampWorldPos = glm::vec3(0.0f, -2.2f, -30.0f);
float           gLampScale = 45.0f;

// Materiales
Material Material_brillante, Material_opaco;

// Timing
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// Iluminación
DirectionalLight mainLight;
PointLight  pointLights[MAX_POINT_LIGHTS];
SpotLight   spotLights[MAX_SPOT_LIGHTS];

// Shaders
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

/*=============== Carro (tu clase) ===============*/
struct CarOffsetsRaw {
    glm::vec3 wheelFL{ 0 }, wheelFR{ 0 }, wheelRL{ 0 }, wheelRR{ 0 };
    glm::vec3 wheelCenter{ 0 };
    glm::vec3 hoodPivot{ 0 };
    glm::vec3 hoodLocal{ 0 };
    glm::vec3 headLLocal{ 0 };
    glm::vec3 headRLocal{ 0 };
    glm::vec3 wsLocal{ 0 };
    float     wsYawDeg{ 0.0f };
};

struct Car {
    Model body, hood, wheel, windshield;

    float     scale = 7.4f;
    glm::vec3 pos = glm::vec3(0);
    float     yawDeg = 0.0f;
    float     wheelAng = 0.0f;
    float     hoodAng = 0.0f;
    float     hoodMax = 50.0f;

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

    void spinWheels(float ddeg) { wheelAng += ddeg; }

    void getHeadlightWorld(bool left, glm::vec3& outPos, glm::vec3& outDir) const {
        glm::mat4 B(1.0f);
        B = glm::translate(B, pos);
        B = glm::rotate(B, glm::radians(yawDeg), glm::vec3(0, 1, 0));
        glm::vec3 loc = left ? comp.HeadL : comp.HeadR;
        outPos = glm::vec3(B * glm::vec4(loc, 1.0f));
        glm::mat4 R(1.0f);
        R = glm::rotate(R, glm::radians(yawDeg), glm::vec3(0, 1, 0));
        outDir = glm::vec3(R * glm::vec4(0, 0, +1, 0));
    }

    void render(GLint uModel) {
        glm::mat4 B(1.0f);
        B = glm::translate(B, pos);
        B = glm::rotate(B, glm::radians(yawDeg), glm::vec3(0, 1, 0));

        // Carrocería
        glm::mat4 M = glm::scale(B, glm::vec3(scale));
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

        auto drawWheel = [&](const glm::vec3& p) {
            glm::mat4 Wm = B;
            Wm = glm::translate(Wm, p);
          
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
        comp.HoodPivot = raw.hoodPivot - raw.hoodLocal;
        comp.WsLocal = raw.wsLocal;
        wsYawDeg = raw.wsYawDeg;
        comp.HeadL = raw.headLLocal;
        comp.HeadR = raw.headRLocal;
    }
};

// promedio de normales
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

/*===================== Objetos (base + flecha/números) =====================*/
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

    // Flecha
    unsigned int flechaIndices[] = { 0,1,2, 0,2,3 };
    GLfloat flechaVertices[] = {
        -0.5f, 0.0f,  0.5f,   0.0f, 0.0f,   0.0f,-1.0f,0.0f,
         0.5f, 0.0f,  0.5f,   1.0f, 0.0f,   0.0f,-1.0f,0.0f,
         0.5f, 0.0f, -0.5f,   1.0f, 1.0f,   0.0f,-1.0f,0.0f,
        -0.5f, 0.0f, -0.5f,   0.0f, 1.0f,   0.0f,-1.0f,0.0f,
    };

    // Tablero números
    unsigned int scoreIndices[] = { 0,1,2, 0,2,3 };
    GLfloat scoreVertices[] = {
        -0.5f, 0.0f,  0.5f,   0.0f, 0.0f,   0.0f,-1.0f,0.0f,
         0.5f, 0.0f,  0.5f,   1.0f, 0.0f,   0.0f,-1.0f,0.0f,
         0.5f, 0.0f, -0.5f,   1.0f, 1.0f,   0.0f,-1.0f,0.0f,
        -0.5f, 0.0f, -0.5f,   0.0f, 1.0f,   0.0f,-1.0f,0.0f,
    };

    // Un número (UV será movido por offset)
    unsigned int numeroIndices[] = { 0,1,2, 0,2,3 };
    GLfloat numeroVertices[] = {
        // x     y     z      u       v        nx    ny    nz
        -0.5f, 0.0f,  0.5f,  0.0f,   0.0f,    0.0f,-1.0f,0.0f,
         0.5f, 0.0f,  0.5f,  0.25f,  0.0f,    0.0f,-1.0f,0.0f,
         0.5f, 0.0f, -0.5f,  0.25f,  0.3333f, 0.0f,-1.0f,0.0f,
        -0.5f, 0.0f, -0.5f,  0.0f,   0.3333f, 0.0f,-1.0f,0.0f,
    };


    Mesh* obj1 = new Mesh(); obj1->CreateMesh(vertices, indices, 32, 12);            meshList.push_back(obj1);
    Mesh* obj2 = new Mesh(); obj2->CreateMesh(vertices, indices, 32, 12);            meshList.push_back(obj2);
    Mesh* obj3 = new Mesh(); obj3->CreateMesh(floorVertices, floorIndices, 32, 6);   meshList.push_back(obj3);
    Mesh* obj4 = new Mesh(); obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12); meshList.push_back(obj4);

    Mesh* obj5 = new Mesh(); obj5->CreateMesh(flechaVertices, flechaIndices, 32, 6); meshList.push_back(obj5); // flecha
    Mesh* obj6 = new Mesh(); obj6->CreateMesh(scoreVertices, scoreIndices, 32, 6);   meshList.push_back(obj6); // tablero
    Mesh* obj7 = new Mesh(); obj7->CreateMesh(numeroVertices, numeroIndices, 32, 6); meshList.push_back(obj7); // un número

    calcAverageNormals(indices, 12, vertices, 32, 8, 5);
    calcAverageNormals(vegetacionIndices, 12, vegetacionVertices, 64, 8, 5);
}

/*===================== D8 =====================*/
void CreateD8()
{
    unsigned int cubo_indices[] = {
        0,1,2,  3,4,5,  6,7,8,  9,10,11,
        12,13,14, 14,15,12,
        16,17,18, 19,20,21, 22,23,24, 25,26,27,
    };
    auto UVtop = [](float cx, float cy) {
        const float du = 0.065f, dv = 0.085f;
        return std::array<glm::vec2, 3>{
            glm::vec2(cx - du, cy - dv),
                glm::vec2(cx + du, cy - dv),
                glm::vec2(cx, cy + dv)
        };
        };
    auto UVbot = [](float cx, float cy) {
        const float du = 0.065f, dv = 0.085f;
        return std::array<glm::vec2, 3>{
            glm::vec2(cx - du, cy + dv),
                glm::vec2(cx + du, cy + dv),
                glm::vec2(cx, cy - dv)
        };
        };

    const glm::vec2 C1 = { 0.50f,0.43f }, C2 = { 0.80f,0.60f }, C3 = { 0.72f,0.30f }, C4 = { 0.50f,0.60f };
    const glm::vec2 C5 = { 0.50f,0.16f }, C6 = { 0.20f,0.60f }, C7 = { 0.28f,0.30f }, C8 = { 0.50f,0.84f };

    auto uv1 = UVtop(C1.x, C1.y), uv2 = UVtop(C2.x, C2.y), uv3 = UVtop(C3.x, C3.y), uv4 = UVtop(C4.x, C4.y);
    auto uv5 = UVbot(C5.x, C5.y), uv6 = UVbot(C6.x, C6.y), uv7 = UVbot(C7.x, C7.y), uv8 = UVbot(C8.x, C8.y);

    GLfloat cubo_vertices[] = {
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

        -0.5f,-0.5f, 0.5f, uv1[0].x, uv1[0].y, 0,1,0,
         0.5f,-0.5f, 0.5f, uv1[1].x, uv1[1].y, 0,1,0,
         0.5f,-0.5f,-0.5f, uv1[1].x, uv1[2].y, 0,1,0,
        -0.5f,-0.5f,-0.5f, uv1[0].x, uv1[2].y, 0,1,0,

        -0.5f,-0.5f, 0.5f, uv5[0].x, uv5[0].y,  0,-1,-1,
         0.5f,-0.5f, 0.5f, uv5[1].x, uv5[1].y,  0,-1,-1,
         0.0f,-1.5f, 0.0f, uv5[2].x, uv5[2].y,  0,-1,-1,

         0.5f,-0.5f, 0.5f, uv6[0].x, uv6[0].y, -1,-1, 0,
         0.5f,-0.5f,-0.5f, uv6[1].x, uv6[1].y, -1,-1, 0,
         0.0f,-1.5f, 0.0f, uv6[2].x, uv6[2].y, -1,-1, 0,

         0.5f,-0.5f,-0.5f, uv7[0].x, uv7[0].y,  0,-1, 1,
        -0.5f,-0.5f,-0.5f, uv7[1].x, uv7[1].y,  0,-1, 1,
         0.0f,-1.5f, 0.0f,  uv7[2].x, uv7[2].y,  0,-1, 1,

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

/*===================== Shaders =====================*/
void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

/*===================== Animación (estado) =====================*/
// Carro
float movCocheZ = 0.0f;      // desplazamiento extra sobre -Z
float carSpeed = 6.0f;       // unidades/seg
bool  carForward = true;     // true = hacia -Z

// Texturas animadas / números
float toffsetflechau = 0.0f, toffsetflechav = 0.0f;
float toffsetnumerou = 0.0f, toffsetnumerov = 0.0f;

// Timers de números
float numLeftTimer = 0.0f;      
float numRightTimer = 0.0f;    
const float numLeftInterval = 0.9f;
const float numRightInterval = 1.2f;

// === Contador para el dígito derecho  ===
int   rightDigitStep = 0;           
float rightDigitTimer = 0.0f;
const float rightDigitInterval = 0.9f;  // igual ritmo que el de la izquierda
const int ATLAS_COLS = 4;
const int ATLAS_ROWS = 3;
const float CELL_U = 1.0f / ATLAS_COLS; // 0.25
const float CELL_V = 1.0f / ATLAS_ROWS; // ~0.3333

// Dragón nuevo (cuerpo + alas)
float dragonTime = 0.0f;        // acumulador de tiempo
float dragonSpeed = 0.8f;       // velocidad del ciclo
float dragonWingAmpDeg = 35.0f; // amplitud del aleteo
float dragonPathHalf = 10.0f;   
glm::vec3 dragonBasePos = glm::vec3(0.0f, 5.0f, 6.0f); // base de vuelo

/*===================== MAIN =====================*/
int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();
    CreateShaders();
    CreateD8();

    
    camera = Camera(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f, 0.0f,
        7.5f,   // moveSpeed -> más rápido 
        0.8f    // turnSpeed
    );

    // Cargas de texturas
    brickTexture = Texture("Textures/brick.png");     brickTexture.LoadTextureA();
    dirtTexture = Texture("Textures/dirt.png");       dirtTexture.LoadTextureA();
    plainTexture = Texture("Textures/plain.png");     plainTexture.LoadTextureA();
    pisoTexture = Texture("Textures/piso.tga");       pisoTexture.LoadTextureA();
    AgaveTexture = Texture("Textures/Agave.tga");     AgaveTexture.LoadTextureA();

    FlechaTexture = Texture("Textures/flechas.tga");        FlechaTexture.LoadTextureA();
    NumerosTexture = Texture("Textures/numerosbase.tga");   NumerosTexture.LoadTextureA();
    Numero1Texture = Texture("Textures/numero1.tga");       Numero1Texture.LoadTextureA();
    Numero2Texture = Texture("Textures/numero2.tga");       Numero2Texture.LoadTextureA();

    PielDragonTexture = Texture("Textures/PielDragon.tga"); PielDragonTexture.LoadTextureA();

    // Modelos
    Kitt_M = Model(); Kitt_M.LoadModel("Models/kitt_optimizado.obj");
    Llanta_M = Model(); Llanta_M.LoadModel("Models/llanta_optimizada.obj");
    Blackhawk_M = Model(); Blackhawk_M.LoadModel("Models/uh60.obj");

    Lampara_M.LoadModel("Models/Lampara.obj");
    StreetLamp_M.LoadModel("Models/StreetLamp.obj");
    ParedLadrillo_M.LoadModel("Models/paredladrillo.obj");
    Crustaceo_M.LoadModel("Models/crustaceo.obj");

    // Dragón nuevo (cuerpo + alas)
    CuerpoDragon_M = Model(); CuerpoDragon_M.LoadModel("Models/Cuerpodragon.obj");
    AlasDragon_M = Model(); AlasDragon_M.LoadModel("Models/Alasdragon.obj");

    // Skybox
    std::vector<std::string> skyboxFaces = {
        "Textures/Skybox/cupertin-lake_rt.tga",
        "Textures/Skybox/cupertin-lake_lf.tga",
        "Textures/Skybox/cupertin-lake_dn.tga",
        "Textures/Skybox/cupertin-lake_up.tga",
        "Textures/Skybox/cupertin-lake_bk.tga",
        "Textures/Skybox/cupertin-lake_ft.tga"
    };
    skybox = Skybox(skyboxFaces);

    // Materiales
    Material_brillante = Material(4.0f, 256);
    Material_opaco = Material(0.3f, 4);

    // Luz direccional
    mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
        0.3f, 0.3f,
        0.0f, 0.0f, -1.0f);

    unsigned int pointLightCount = 0;
    // Puntual morada (crustáceo)
    const int CRUST_POINT = pointLightCount;
    pointLights[pointLightCount++] = PointLight(
        0.60f, 0.20f, 0.80f, 0.10f, 2.2f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.022f, 0.0019f
    );

    // Puntual roja
    pointLights[pointLightCount++] = PointLight(
        1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -6.0f, 1.5f, 1.5f, 1.0f, 0.2f, 0.1f
    );

    unsigned int spotLightCount = 0;

    // 0) Linterna cámara
    const int camSpotIdx = spotLightCount;
    spotLights[spotLightCount++] = SpotLight(
        1.0f, 1.0f, 1.0f, 0.0f, 2.0f,
        0, 0, 0, 0, -1, 0, 1.0f, 0.09f, 0.032f, 12.0f
    );

    // 1) Verde fija de prueba
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
    car.setBasePosition(glm::vec3(0.0f, -2.2f, -14.0f)); // base

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

    // Faros del coche
    const int HL_L = spotLightCount++;
    const int HL_R = spotLightCount++;
    spotLights[HL_L] = SpotLight(0.30f, 0.65f, 1.00f, 0.06f, 10.0f, 0, 0, 0, 0, 0, +1, 1.0f, 0.009f, 0.00032f, 24.0f);
    spotLights[HL_R] = SpotLight(0.30f, 0.65f, 1.00f, 0.06f, 10.0f, 0, 0, 0, 0, 0, +1, 1.0f, 0.009f, 0.00032f, 24.0f);

    // Spotlight cofre (verde)
    const int HOOD_SPOT = spotLightCount++;
    spotLights[HOOD_SPOT] = SpotLight(0.10f, 0.95f, 0.10f, 0.05f, 2.2f, 0, 0, 0, 0, 0, +1, 1.0f, 0.009f, 0.00032f, 20.0f);

    // Spotlights movimiento (frente/atrás)
    const int MOVE_FWD_SPOT = spotLightCount++;
    const int MOVE_BACK_SPOT = spotLightCount++;
    spotLights[MOVE_FWD_SPOT] = SpotLight(1.00f, 0.55f, 0.10f, 0.04f, 1.8f, 0, 0, 0, 0, 0, +1, 1.0f, 0.009f, 0.00032f, 18.0f);
    spotLights[MOVE_BACK_SPOT] = SpotLight(1.00f, 1.00f, 0.10f, 0.04f, 1.6f, 0, 0, 0, 0, 0, -1, 1.0f, 0.009f, 0.00032f, 18.0f);

    // Helicóptero (opcional)
    struct HelicopterState { glm::vec3 pos{ -8.0f,6.0f,-12.0f }; float yawDeg{ 0 }; float speed{ 4.0f }; int spotIdx{ -1 }; } heli;
    {
        const GLfloat edgeDeg = 30.0f;
        int idx = spotLightCount++;
        heli.spotIdx = idx;
        spotLights[idx] = SpotLight(1.0f, 0.95f, 0.60f, 0.15f, 4.0f,
            heli.pos.x, heli.pos.y, heli.pos.z,
            0.0f, -1.0f, 0.0f, 1.0f, 0.045f, 0.0072f, edgeDeg);
    }

    // Lámparas
    LampState lamp;
    struct StreetLampState { int pointIdx{ -1 }; } street;

    // Lámpara pequeña
    {
        const glm::vec3 lampBulbLocal(0.0f, 1.0f, 0.0f);
        glm::vec3 bulb = glm::vec3(glm::scale(glm::translate(glm::mat4(1.0f), gLampWorldPos), glm::vec3(gLampScale)) * glm::vec4(lampBulbLocal, 1.0f));
        lamp.bulbPos = bulb;
        pointLights[pointLightCount] = PointLight(
            1.0f, 0.9f, 0.6f, 0.3f, 3.0f,
            bulb.x, bulb.y, bulb.z,
            1.0f, 0.02f, 0.001f
        );
        lamp.pointIdx = pointLightCount;
        pointLightCount++;
    }
    // StreetLamp
    {
        street.pointIdx = pointLightCount;
        pointLights[pointLightCount] = PointLight(
            1.0f, 1.0f, 1.0f, 0.12f, 2.4f,
            streetLampPos.x, streetLampPos.y + kLampScale * 13.9f, streetLampPos.z,
            1.0f, 0.022f, 0.0019f
        );
        pointLightCount++;
    }

    // Pared / Crustáceo
    const float floorScale = 30.0f;               // escala del piso
    const float floorHalf = 10.0f * floorScale;  // 300
    const glm::vec3 wallPos = glm::vec3(0.0f, 0.0f, -14.0f) + glm::vec3(0.0f, 0.0f, +45.0f);
    const glm::mat4 paredModel = glm::scale(
        glm::translate(glm::mat4(1.0f), wallPos + glm::vec3(0.0f, -2.2f, 0.0f)),
        glm::vec3(5.0f)
    );

    const glm::vec3 crustPos = glm::vec3(100.0f, -2.2f, -18.0f);
    const glm::mat4 crustModel = glm::scale(
        glm::translate(glm::mat4(1.0f), crustPos),
        glm::vec3(10.8f)
    );

    // Uniforms
    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
        uniformSpecularIntensity = 0, uniformShininess = 0, uniformColor = 0, uniformTextureOffset = 0;

    glm::mat4 projection = glm::perspective(45.0f,
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 2000.0f);

    float prevMovZ = movCocheZ;

    while (!mainWindow.getShouldClose())
    {
        // === Delta time (simple) ===
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        // === Animación Carro sobre -Z con límites del piso ===
        const float baseZ = -14.0f;
        const float zMin = -floorHalf + 5.0f; // margen
        const float zMax = baseZ;
        if (carForward) {
            movCocheZ += carSpeed * deltaTime;
            if (baseZ - movCocheZ <= zMin) { movCocheZ = baseZ - zMin; carForward = false; }
        }
        else {
            movCocheZ -= carSpeed * deltaTime;
            if (baseZ - movCocheZ >= zMax) { movCocheZ = baseZ - zMax; carForward = true; }
        }
        car.setBasePosition(glm::vec3(0.0f, -2.2f, baseZ - movCocheZ));

        // *** Llantas fijas: no actualizar ángulo de rueda ***

        // === Dragón (cuerpo + aleteo) ===
        dragonTime += deltaTime * dragonSpeed;
        float t = dragonTime;
        float wingAngle = dragonWingAmpDeg * sinf(6.0f * t); // aleteo
        glm::vec3 dPos = dragonBasePos
            + glm::vec3(dragonPathHalf * sinf(t), 0.6f * sinf(2.0f * t), 0.0f);

        // === Timers de números (más lentos) ===
        numLeftTimer += deltaTime;
        numRightTimer += deltaTime; // reservado

        

        // Entrada usuario
        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        // === Render ===
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
        uniformTextureOffset = shaderList[0].getOffsetLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

        // Linterna cámara
        glm::vec3 lowerLight = camera.getCameraPosition(); lowerLight.y -= 0.3f;
        spotLights[camSpotIdx].SetFlash(lowerLight, camera.getCameraDirection());

        // Faros coche
        {
            glm::vec3 pL, dL, pR, dR;
            car.getHeadlightWorld(true, pL, dL);
            car.getHeadlightWorld(false, pR, dR);
            const glm::vec3 down(0.0f, -0.25f, 0.0f);
            dL = glm::normalize(dL + down);
            dR = glm::normalize(dR + down);
            spotLights[HL_L].SetFlash(pL, dL);
            spotLights[HL_R].SetFlash(pR, dR);
        }

        // Spotlight cofre
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

            spotLights[HOOD_SPOT] = SpotLight(
                0.10f, 0.95f, 0.10f, 0.05f, 2.2f,
                worldPos.x, worldPos.y, worldPos.z,
                worldDir.x, worldDir.y, worldDir.z,
                1.0f, 0.009f, 0.00032f, 20.0f
            );
        }

        // Spotlights de movimiento (frente/atrás)
        {
            float dm = movCocheZ - prevMovZ;
            prevMovZ = movCocheZ;

            bool advancing = (dm > 0.0005f);
            bool reversing = (dm < -0.0005f);

            glm::vec3 pL, dL, pR, dR;
            car.getHeadlightWorld(true, pL, dL);
            car.getHeadlightWorld(false, pR, dR);

            const glm::vec3 frontPos = (pL + pR) * 0.5f;
            glm::vec3 baseDir = glm::normalize((dL + dR) * 0.5f);
            glm::vec3 frontDir = -baseDir, backDir = baseDir;
            const float backOffset = 6.0f;
            const glm::vec3 backPos = frontPos - frontDir * backOffset;

            const glm::vec3 down(0.0f, -0.25f, 0.0f);
            frontDir = glm::normalize(frontDir + down);
            backDir = glm::normalize(backDir + down);

            spotLights[MOVE_FWD_SPOT] = SpotLight(
                1.00f, 0.55f, 0.10f, advancing ? 0.05f : 0.0f, advancing ? 1.8f : 0.0f,
                frontPos.x, frontPos.y, frontPos.z,
                frontDir.x, frontDir.y, frontDir.z,
                1.0f, 0.009f, 0.00032f, 18.0f
            );
            spotLights[MOVE_BACK_SPOT] = SpotLight(
                1.00f, 1.00f, 0.10f, reversing ? 0.05f : 0.0f, reversing ? 1.6f : 0.0f,
                backPos.x, backPos.y, backPos.z,
                backDir.x, backDir.y, backDir.z,
                1.0f, 0.009f, 0.00032f, 18.0f
            );
        }

        // Toggle lámpara pequeña
        mainWindow.ProcessLampToggle(lamp, deltaTime);
        if (lamp.pointIdx >= 0) {
            const int i = lamp.pointIdx;
            pointLights[i] = PointLight(1.0f, 0.9f, 0.6f,
                lamp.on ? 0.3f : 0.0f,
                lamp.on ? 3.0f : 0.0f,
                lamp.bulbPos.x, lamp.bulbPos.y, lamp.bulbPos.z,
                1.0f, 0.02f, 0.001f);
        }

        // StreetLamp
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

        // Subir luces
        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, pointLightCount);
        shaderList[0].SetSpotLights(spotLights, spotLightCount);

        glm::mat4 model(1.0f);
        glm::vec3 color(1.0f);

        // Piso
        glm::vec2 tOffset(0.0f, 0.0f);
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(tOffset));
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(floorScale, 1.0f, floorScale));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        pisoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[2]->RenderMesh();

        // Carro
        car.render(uniformModel);

        // Helicóptero
        model = glm::mat4(1.0f);
        model = glm::translate(model, heli.pos);
        model = glm::rotate(model, glm::radians(heli.yawDeg), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.3f));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(1, 0, 0));
        model = glm::rotate(model, 90 * toRadians, glm::vec3(0, 0, 1));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Blackhawk_M.RenderModel();

        // Lámpara pequeña
        const glm::mat4 lamparaModelR =
            glm::scale(glm::translate(glm::mat4(1.0f), gLampWorldPos), glm::vec3(gLampScale));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(lamparaModelR));
        Lampara_M.RenderModel();

        // StreetLamp
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(streetLampModelR));
        StreetLamp_M.RenderModel();

        // Agave (alpha)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, -4.0f));
        model = glm::scale(model, glm::vec3(4.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        AgaveTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[3]->RenderMesh();

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

        /* =================== Dragón (Cuerpo + 1 malla de Alas) =================== */
        {
            // Cuerpo
            glm::mat4 M = glm::mat4(1.0f);
            M = glm::translate(M, dPos);
            M = glm::scale(M, glm::vec3(0.3f));
            M = glm::rotate(M, -90.0f * toRadians, glm::vec3(1, 0, 0)); 
            M = glm::rotate(M, +180.0f * toRadians, glm::vec3(0, 1, 0)); 
            M = glm::rotate(M, -90.0f * toRadians, glm::vec3(1, 0, 0));  
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(M));
            PielDragonTexture.UseTexture();
            Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
            CuerpoDragon_M.RenderModel();

            // Alas
            glm::mat4 WA = glm::mat4(1.0f);
            WA = glm::translate(WA, dPos);
            WA = glm::scale(WA, glm::vec3(0.3f));
            WA = glm::rotate(WA, -90.0f * toRadians, glm::vec3(1, 0, 0));
            WA = glm::rotate(WA, +180.0f * toRadians, glm::vec3(0, 1, 0));
            WA = glm::rotate(WA, -90.0f * toRadians, glm::vec3(1, 0, 0)); 
            WA = glm::rotate(WA, wingAngle * toRadians, glm::vec3(0, 0, 1));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(WA));
            PielDragonTexture.UseTexture();
            Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
            AlasDragon_M.RenderModel();
        }

        /* =================== Texturas animadas y NÚMEROS =================== */

        // Flecha con desplazamiento U
        toffsetflechau += 0.001f; if (toffsetflechau > 1.0f) toffsetflechau = 0.0f;
        toffsetflechav = 0.0f;
        tOffset = glm::vec2(toffsetflechau, toffsetflechav);

        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -6.0f));
        model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(tOffset));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(1.0f, 0.0f, 0.0f)));
        FlechaTexture.UseTexture();
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[4]->RenderMesh();

        // Tablero con todos los números
        toffsetnumerou = 0.0f; toffsetnumerov = 0.0f;
        tOffset = glm::vec2(toffsetnumerou, toffsetnumerov);
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-6.0f, 2.0f, -6.0f));
        model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(tOffset));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(1.0f)));
        NumerosTexture.UseTexture();
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[5]->RenderMesh();

        // ======= Arriba izquierda: alterna 1 ↔ 2 (texturas) =======
        if (numLeftTimer >= numLeftInterval) numLeftTimer = 0.0f;
        {
            bool showTwo = (numLeftTimer > numLeftInterval * 0.5f);
            glm::vec2 tOffsetL(0.0f, 0.0f);
            model = glm::mat4(1.0);
            model = glm::translate(model, glm::vec3(-10.0f, 10.0f, -6.0f));
            model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
            glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(tOffsetL));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(1.0f)));
            if (showTwo) Numero2Texture.UseTexture(); else Numero1Texture.UseTexture();
            Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
            meshList[5]->RenderMesh(); // usamos el mismo quad grande
        }

        // ======= Arriba derecha: dígito único que cicla 1..9,0 =======
        {
            // Timer ÚNICO y estable para este dígito
            static float acc = 0.0f;
            static int   step = 0;             
            const  float INTERVAL = 0.8f;      // segundos por dígito 

            acc += deltaTime;
            while (acc >= INTERVAL) {          
                acc -= INTERVAL;
                step = (step + 1) % 10;
            }

            // Dígito a mostrar: 1..9,0
            int digit = (step == 9) ? 0 : (step + 1);        
            int atlasIndex = (digit == 0) ? 9 : (digit - 1); 
            // Atlas 4x3
            const int   ATLAS_COLS = 4;
            const int   ATLAS_ROWS = 3;
            const float CELL_U = 1.0f / ATLAS_COLS;   
            const float CELL_V = 1.0f / ATLAS_ROWS;  

            int col = atlasIndex % ATLAS_COLS;             
            int rowTopDown = atlasIndex / ATLAS_COLS;      
            int rowUV = (ATLAS_ROWS - 1) - rowTopDown;     

            // --- Offset UV a la celda deseada
            glm::vec2 tOffsetR(col* CELL_U, rowUV* CELL_V);
            glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(tOffsetR));

            // --- Dibujo con el QUAD DE UN NÚMERO
            glm::mat4 modelR = glm::mat4(1.0f);
            modelR = glm::translate(modelR, glm::vec3(-7.0f, 10.0f, -6.0f));
            modelR = glm::rotate(modelR, 90 * toRadians, glm::vec3(1, 0, 0));
            modelR = glm::scale(modelR, glm::vec3(3.0f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelR));

            NumerosTexture.UseTexture();          
            Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
            meshList[6]->RenderMesh();            

        }

        glDisable(GL_BLEND);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}
