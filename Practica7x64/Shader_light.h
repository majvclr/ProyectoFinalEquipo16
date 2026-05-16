#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>

#include <glew.h>

#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

class Shader
{
public:
    Shader();

    void CreateFromString(const char* vertexCode, const char* fragmentCode);
    void CreateFromFiles(const char* vertexLocation, const char* fragmentLocation);

    std::string ReadFile(const char* fileLocation);

    // --- AQUÍ ESTÁ EL MÉTODO NUEVO ---
    GLuint GetProgramID() { return shaderID; }

    GLuint GetProjectionLocation();
    GLuint GetModelLocation();
    GLuint GetViewLocation();
    GLuint GetAmbientIntensityLocation();
    GLuint GetAmbientcolorLocation();
    GLuint GetDiffuseIntensityLocation();
    GLuint GetDirectionLocation();
    GLuint GetSpecularIntensityLocation();
    GLuint GetShininessLocation();
    GLuint GetEyePositionLocation();
    GLuint getColorLocation();
    GLuint getOffsetLocation();
    GLuint getSubSizeLocation();   // <-- NUEVO

    void SetDirectionalLight(DirectionalLight* dLight);
    void SetPointLights(PointLight* pLight, unsigned int lightCount);
    void SetSpotLights(SpotLight* sLight, unsigned int lightCount);

    void UseShader();
    void ClearShader();

    ~Shader();

private:
    int pointLightCount;
    int spotLightCount;

    // AQUI agregamos uniformTextureSubSize
    GLuint shaderID,
        uniformProjection,
        uniformModel,
        uniformColor,
        uniformView,
        uniformEyePosition,
        uniformSpecularIntensity,
        uniformShininess,
        uniformTextureOffset,
        uniformTextureSubSize;   // <-- NUEVO

    struct {
        GLuint uniformcolor;
        GLuint uniformAmbientIntensity;
        GLuint uniformDiffuseIntensity;

        GLuint uniformDirection;
    } uniformDirectionalLight;

    GLuint uniformPointLightCount;

    struct {
        GLuint uniformcolor;
        GLuint uniformAmbientIntensity;
        GLuint uniformDiffuseIntensity;

        GLuint uniformPosition;
        GLuint uniformConstant;
        GLuint uniformLinear;
        GLuint uniformExponent;
    } uniformPointLight[MAX_POINT_LIGHTS];

    GLuint uniformSpotLightCount;

    struct {
        GLuint uniformColour;
        GLuint uniformAmbientIntensity;
        GLuint uniformDiffuseIntensity;

        GLuint uniformPosition;
        GLuint uniformConstant;
        GLuint uniformLinear;
        GLuint uniformExponent;

        GLuint uniformDirection;
        GLuint uniformEdge;
    } uniformSpotLight[MAX_SPOT_LIGHTS];

    void CompileShader(const char* vertexCode, const char* fragmentCode);
    void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType);
};