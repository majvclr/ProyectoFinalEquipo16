#pragma once
#include <glew.h>

class Texture
{
public:
	Texture();
	Texture(const char* FileLoc);
	bool LoadTexture();
	bool LoadTextureA();
	void UseTexture();
	void ClearTexture();
    
	// Método público añadido para exponer el ID privado de la textura
	GLuint GetID() { return textureID; }

	~Texture();
private: 
	GLuint textureID;
	int width, height, bitDepth;
	const char *fileLocation;
};