#include "Model.h"
#include <iostream>

Model::Model() {}

void Model::LoadModel(const std::string & fileName)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
    
    if (!scene)
    {	
        printf("Fallo en cargar el modelo: %s \n", fileName.c_str());
        return;
    }

    LoadNode(scene->mRootNode, scene);
    LoadMaterials(scene);
}

void Model::LoadMesh(aiMesh * mesh, const aiScene * scene)
{
    std::vector<GLfloat> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        // Posiciones
        vertices.insert(vertices.end(), { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
        
        // Coordenadas UV
        if (mesh->mTextureCoords[0])
        {
            vertices.insert(vertices.end(), { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
        }
        else
        {
            vertices.insert(vertices.end(), { 0.0f, 0.0f });
        }
        
        // NORMALES CORREGIDAS (Sin signo negativo)
        vertices.insert(vertices.end(), { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z });
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    Mesh* newMesh = new Mesh();
    newMesh->CreateMesh(&vertices[0], &indices[0], vertices.size(), indices.size());
    MeshList.push_back(newMesh);
    meshTotex.push_back(mesh->mMaterialIndex);
}

void Model::LoadMaterials(const aiScene * scene)
{
    TextureList.resize(scene->mNumMaterials);
    for (unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        aiMaterial* material = scene->mMaterials[i];
        TextureList[i] = nullptr;

        if (material->GetTextureCount(aiTextureType_DIFFUSE))
        {
            aiString path;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
            {
                std::string fullPath = std::string(path.data);
                size_t idx = fullPath.find_last_of("\\/");
                std::string filename = (idx == std::string::npos) ? fullPath : fullPath.substr(idx + 1);
                
                std::string texPath = std::string("Textures/") + filename;
                TextureList[i] = new Texture(texPath.c_str());

                if (!TextureList[i]->LoadTexture())
                {
                    printf("Fallo en cargar la Textura: %s, usando plain.png\n", texPath.c_str());
                    delete TextureList[i];
                    TextureList[i] = nullptr;
                }
            }
        }

        if (!TextureList[i])
        {
            TextureList[i] = new Texture("Textures/plain.png");
            TextureList[i]->LoadTexture();
        }
    }
}

void Model::RenderModel()
{
    for (unsigned int i = 0; i < MeshList.size(); i++)
    {
        unsigned int materialIndex = meshTotex[i];
        if (materialIndex < TextureList.size() && TextureList[materialIndex])
        {
            TextureList[materialIndex]->UseTexture();
        }
        MeshList[i]->RenderMesh();
    }
}

void Model::LoadNode(aiNode * node, const aiScene * scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        LoadNode(node->mChildren[i], scene);
    }
}

void Model::ClearModel()
{
    for (auto& mesh : MeshList) { if (mesh) delete mesh; }
    MeshList.clear();
    for (auto& tex : TextureList) { if (tex) delete tex; }
    TextureList.clear();
}

Model::~Model() { ClearModel(); }