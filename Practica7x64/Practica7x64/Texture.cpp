#include "Texture.h"
#include "CommonValues.h"

Texture::Texture()
{
    textureID = 0;
    width = 0;
    height = 0;
    bitDepth = 0;
    fileLocation = nullptr;
}

Texture::Texture(const char* FileLoc)
{
    textureID = 0;
    width = 0;
    height = 0;
    bitDepth = 0;
    fileLocation = FileLoc;
}

// Función unificada para cargar texturas con o sin canal Alfa (JPG/PNG)
bool Texture::LoadTexture()
{
    // Cambiamos el origen a la esquina inferior izquierda como requiere OpenGL
    stbi_set_flip_vertically_on_load(true);

    // Forzamos STBI_rgb_alpha para que cualquier imagen sea interpretada como RGBA (4 canales)
    // Esto evita errores de memoria al leer imágenes de 24 bits (JPG) o 32 bits (PNG)
    unsigned char* texData = stbi_load(fileLocation, &width, &height, &bitDepth, STBI_rgb_alpha);

    if (!texData)
    {
        printf("Error: No se encontro el archivo o fallo la carga en: %s\n", fileLocation);
        return false;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Configuración de repetición (Wrap)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Filtros de suavizado (Mipmaps para evitar el efecto de "parpadeo" en texturas lejanas)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Enviamos los datos a la GPU usando el formato GL_RGBA por el forzado anterior
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind
    stbi_image_free(texData);       // Liberamos memoria del CPU

    return true;
}

// Mantenemos LoadTextureA por compatibilidad, pero ahora hace lo mismo que LoadTexture
bool Texture::LoadTextureA()
{
    return LoadTexture();
}

void Texture::UseTexture()
{
    // Activamos la unidad de textura 0 por defecto
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::ClearTexture()
{
    if (textureID != 0)
    {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
    width = 0;
    height = 0;
    bitDepth = 0;
    fileLocation = nullptr;
}

Texture::~Texture()
{
    ClearTexture();
}