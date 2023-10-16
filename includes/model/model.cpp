#include "model.h"
#include <iostream>
#include "codeConvert.h"

void Model::Draw(unsigned int ID)
{
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(ID);
}

void Model::loadModel(string path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);    

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    // 处理节点所有的网格（如果有的话）
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes.push_back(processMesh(mesh, scene));         
    }
    // 接下来对它的子节点重复这一过程
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    // data to fill
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    // walk through each of the mesh's vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        // texture coordinates
        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            if (mesh->mTangents != NULL)
            {
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
            }
            if (mesh->mBitangents != NULL)
            {
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);        
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    bool heightNormalFlag = false;
    bool normalFlag = false;
    if (normalMaps.size() > 0)
        heightNormalFlag = true;
    std::vector<Texture> normalMaps1 = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
    if (normalMaps1.size() > 0)
        normalFlag = true;
    if (normalMaps.size() > 0 && heightNormalFlag && normalFlag)
        std::cout << "[Model::processMesh] warning: aiTextureType_HEIGHT and aiTextureType_NORMALS both are recognize as normalMaps!" << std::endl;
    if (normalMaps.size() > 0 && normalMaps1 == normalMaps)
        std::cout << "[Model::processMesh] warning: aiTextureType_NORMALS has been loaded repeatly as " << normalMaps[0].type << std::endl;
    else
        textures.insert(textures.end(), normalMaps1.begin(), normalMaps1.end());
    // 4. height maps
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    // 5. PBR albedo
    vector<bool> bRepeats;
    std::vector<Texture> albedoMaps = loadMaterialTextures(material, aiTextureType_BASE_COLOR, "texture_albedo", &bRepeats);
    for (size_t i = 0; i < albedoMaps.size(); i++)
    {
        if (bRepeats[i])//避免重复添加
        {
            std::cout << "[Model::processMesh] warning: aiTextureType_BASE_COLOR has been loaded repeatly as " << albedoMaps[i].type << std::endl;
            albedoMaps.erase(albedoMaps.begin() + i);
            bRepeats.erase(bRepeats.begin() + i);
            --i;
        }
    }
    textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());
    // 6. PBR metallicMap
    std::vector<Texture> metallicMaps = loadMaterialTextures(material, aiTextureType_METALNESS, "texture_metallic");
    textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
    // 7. PBR roughnessMap
    std::vector<Texture> roughnessMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness", &bRepeats);
    for (size_t i = 0; i < roughnessMaps.size(); i++)
    {
        if (bRepeats[i])//避免重复添加
        {
            std::cout << "[Model::processMesh] warning: aiTextureType_DIFFUSE_ROUGHNESS has been loaded repeatly as " << roughnessMaps[i].type << std::endl;
            roughnessMaps.erase(roughnessMaps.begin() + i);
            bRepeats.erase(bRepeats.begin() + i);
            --i;
        }
    }
    textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
    // 8. PBR aoMap
    std::vector<Texture> aoMaps = loadMaterialTextures(material, aiTextureType_UNKNOWN, "texture_ao", &bRepeats);
    for (size_t i = 0; i < aoMaps.size(); i++)
    {
        if (bRepeats[i])//避免重复添加
        {
            std::cout << "[Model::processMesh] warning: aiTextureType_UNKNOWN has been loaded repeatly as " << aoMaps[i].type << std::endl;
            aoMaps.erase(aoMaps.begin() + i);
            bRepeats.erase(bRepeats.begin() + i);
            --i;
        }
    }
    textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
    // 9. other maps
    for (int j = aiTextureType_EMISSIVE; j < AI_TEXTURE_TYPE_MAX; j++)
    {
        if (j == aiTextureType_HEIGHT)
            continue;
        std::vector<Texture> otherMaps = loadMaterialTextures(material, aiTextureType(j), "texture_other");
        if (otherMaps.size() > 0)
            std::cout << "[Model::processMesh] Unknown Map: " << j << std::endl;
    }
    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, textures);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName, vector<bool> *bRepeats/* = nullptr*/)
{
    if (bRepeats != nullptr)
        bRepeats->clear();
    vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        //string strUtf8(str.C_Str());
        //string strFromUtf8 = UTF8ToANSI(strUtf8);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if(!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
        }
        if (bRepeats != nullptr)
           bRepeats->push_back(skip);
    }
    return textures;
}

unsigned int Model::TextureFromFile(const char *path, const string &directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
