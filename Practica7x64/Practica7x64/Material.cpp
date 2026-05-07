#include "Material.h"

Material::Material()
{
	specularIntensity = 0.0f;
	shininess = 0.0f;
}

Material::Material(GLfloat sIntensity, GLfloat shine)
{
	specularIntensity = sIntensity;
	shininess = shine;
}

void Material::UseMaterial(GLuint specularIntensityLocation, GLuint shininessLocation)
{
    // Enviamos los valores de brillo al Fragment Shader
	glUniform1f(specularIntensityLocation, specularIntensity);
	glUniform1f(shininessLocation, shininess);
}

Material::~Material() {}