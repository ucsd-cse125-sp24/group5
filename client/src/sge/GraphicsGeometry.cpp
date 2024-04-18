//
// Created by benjx on 4/8/2024.
//

#include "sge/GraphicsGeometry.h"

#define STB_IMAGE_IMPLEMENTATION // Needed for stb_image.h
#define DISTANCE_FROM_PLAYER 10.0f
#include <stb_image.h>

/**
 * Shitty graphics engine (SGE)
 */
namespace sge {

    glm::vec3 ModelComposite::cameraPosition;
    glm::vec3 ModelComposite::cameraDirection;
    glm::vec3 ModelComposite::cameraUp;

    /**
     * Create a ModelComposite (A 3d object model composed of mesh(es))
     * @param filename Path to .obj file specifying ModelComposite
     */
    sge::ModelComposite::ModelComposite(std::string filename) {
        // Load model
        parentDirectory = std::filesystem::path(filename).remove_filename();
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(filename,
                                                 ASSIMP_IMPORT_FLAGS);

        // Allocate space for meshes, vertices, normals, etc.
        meshes.reserve(scene->mNumMeshes);
        materials.reserve(scene->mNumMaterials);
        reserveGeometrySpace(scene);

        // Load meshes into ModelComposite data structures
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            loadMesh(*scene->mMeshes[i]);
        }

        // TODO: load textures/add textures to loadMaterials
        loadMaterials(scene);
        initBuffers();
        importer.FreeScene();
    }

    sge::ModelComposite::~ModelComposite() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(NUM_BUFFERS, buffers);
        // TODO: also delete textures somewhere, not in this destructor because texID's are shared across all modelComposites
    }

    /**
     * Populate ModelComposite data structures with mesh data
     * @param mesh Mesh to load
     */
    void ModelComposite::loadMesh(aiMesh &mesh) {
        assert(mesh.mNormals != nullptr);
        assert(mesh.mVertices != nullptr);
        // Add vertices, normals, texture coords, etc. to ModelComposite data structures
        for (unsigned int i = 0; i < mesh.mNumVertices; i++) {
            const aiVector3D &vertex = mesh.mVertices[i];
            vertices.push_back(glm::vec3(vertex[0], vertex[1], vertex[2]));
            const aiVector3D &normal = mesh.mNormals[i];
            normals.push_back(glm::vec3(normal[0], normal[1], normal[2]));
            if (mesh.HasTextureCoords(0)) {
                //  Third coordinate is always 0 (Because Assimp is dumb), we can ignore it
                const aiVector3D &texcoord = mesh.mTextureCoords[0][i];
                texcoords.push_back(glm::vec2(texcoord[0], texcoord[1]));
            } else {
                texcoords.push_back(glm::vec2(0));
            }
        }
        // TODO: load bones

        // Load faces/triangles into indices
        for (unsigned int i = 0; i < mesh.mNumFaces; i++) {
            const aiFace &face = mesh.mFaces[i];
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }
    }

    /**
     * Initialize OpenGL buffers for ModelComposite
     */
    void ModelComposite::initBuffers() {
        glGenVertexArrays(1, &VAO);

        glBindVertexArray(VAO);

        glGenBuffers(NUM_BUFFERS, buffers);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTEX_BUF]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(VERTEX_POS);
        glVertexAttribPointer(VERTEX_POS, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[NORMAL_BUF]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), &normals[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(NORMAL_POS);
        glVertexAttribPointer(NORMAL_POS, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[TEXCOORD_BUF]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords[0]) * texcoords.size(), &texcoords[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(TEXCOORD_POS);
        glVertexAttribPointer(TEXCOORD_POS, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_BUF]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    /**
     * Allocate enough space for all vertices, normals, texture coordinates, etc for ModelComposite
     * and add individual meshes to "meshes" vector
     */
    void ModelComposite::reserveGeometrySpace(const aiScene *scene) {
        assert(scene != nullptr);
        unsigned int num_vertex = 0;
        unsigned int num_indices = 0;
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            const aiMesh &mesh = *scene->mMeshes[i];
            // NumIndices = 3 * NumFaces as each face is a triangle, which consists of 3 vertices. Each index references a specific vertex
            meshes.push_back(Mesh(mesh.mNumFaces * 3, num_vertex, num_indices, mesh.mMaterialIndex));
            num_vertex += mesh.mNumVertices;
            num_indices += meshes.back().NumIndices;
        }
        vertices.reserve(num_vertex);
        normals.reserve(num_vertex);
        indices.reserve(num_indices);
        texcoords.reserve(num_vertex);
        if (scene->HasAnimations()) {
            // TODO: fill in
            // bones.reserve(num_indices); or something idk
        }
    }

    // prolly not the right place to add it. feel free to move around
    void ModelComposite::updateCameraToFollowPlayer(glm::vec3 playerPosition, float yaw, float pitch) {
        // the camera and the player should face the same direction (?)
        cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraDirection.y = sin(glm::radians(pitch));
        cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        // the camera is D distance behind the player
        cameraPosition = playerPosition - (cameraDirection * DISTANCE_FROM_PLAYER);

        // update camera's up
        cameraUp = glm::cross(glm::cross(cameraDirection, glm::vec3(0, 1, 0)), cameraDirection);
    }

    void ModelComposite::render(glm::vec3 modelPosition, float modelYaw) const {
        glUseProgram(sge::program);
        glBindVertexArray(VAO);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), modelPosition);
        model = glm::rotate(model, glm::radians(modelYaw), glm::vec3(0.0f, -1.0f, 0.0f));
        // yaw (cursor movement) should rotate our player model AND the camera view, right? 
        // glm::mat4 modelview = glm::perspective(glm::radians(90.0f), (float)sge::windowWidth / (float)sge::windowHeight, 0.5f, 1000.0f) * glm::lookAt(glm::vec3(10, 10, -5), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0, 1, 0)) * model;
        glm::mat4 modelview = glm::perspective(glm::radians(90.0f), (float)sge::windowWidth / (float)sge::windowHeight, 0.5f, 1000.0f) * glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp) * model;
        glUniformMatrix4fv(sge::modelViewPos, 1, GL_FALSE, &modelview[0][0]);
        for (unsigned int i = 0; i < meshes.size(); i++) {
            const Material &mat = materials[meshes[i].MaterialIndex];
            if (mat.diffuseMap != -1) {
                // Tell shader there is a diffuse map
                glUniform1i(sge::hasDiffuseTexture, 1);
                glActiveTexture(GL_TEXTURE0 + textures[mat.diffuseMap].type);
                glBindTexture(GL_TEXTURE_2D, texID[mat.diffuseMap]);
                // todo: only need to redo texsampler stuff for different shader programs
            } else {
                // Tell shader there is no diffuse map
                glUniform1i(sge::hasDiffuseTexture, 0);
                glUniform3fv(sge::diffuseColor, 1, &mat.diffuse[0]);
            }
            glDrawElementsBaseVertex(GL_TRIANGLES, meshes[i].NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * meshes[i].BaseIndex), meshes[i].BaseVertex);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glBindVertexArray(0);
    }

    /**
     * Loads all material properties from a scene into ModelComposite material vector
     * @param scene Scene to load materials from
     */
    void ModelComposite::loadMaterials(const aiScene *scene) {
        for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
            const aiMaterial &mat = *scene->mMaterials[i];
            aiColor4D color(0.f, 0.f, 0.f, 0.0f);
            int shadingModel = 0;

            glm::vec3 diffuse(0.5f);
            glm::vec3 specular(0.0f);
            glm::vec3 emissive(0.0f);
            glm::vec3 ambient(0.0f);

            if (mat.Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
                diffuse.x = color.r;
                diffuse.y = color.g;
                diffuse.z = color.b;
            }
            if (mat.Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
                specular.x = color.r;
                specular.y = color.g;
                specular.z = color.b;
            }
            if (mat.Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
                emissive.x = color.r;
                emissive.y = color.g;
                emissive.z = color.b;
            }
            if (mat.Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
                ambient.x = color.r;
                ambient.y = color.g;
                ambient.z = color.b;
            }

            int idx = loadTexture(aiTextureType_DIFFUSE, scene, mat);

            materials.push_back(Material(specular, emissive, ambient, diffuse, idx));
        }
    }

    /**
     * Loads a texture into SGE
     * @param type Type of texture to load e.g. specular, diffuse, etc.
     * @param scene Assimp scene object we're loading textures from
     * @param material Material we're loading texture for
     * @return Index of new texture within global textures vector
     */
    int ModelComposite::loadTexture(aiTextureType type, const aiScene *scene, const aiMaterial &material) {
        // Load textures
        aiString path;

        if (material.GetTexture(type, 0, &path) != AI_SUCCESS) {
//            std::cout << "uh oh, no diffuse texture\n";
            return -1;
        }

        // Build path to texture file
        std::string textureRelativePath(path.C_Str());
        std::string textureAbsolutePath = parentDirectory.string() + textureRelativePath;

        // If we've already loaded the texture, no need to load it again
        if (textureIdx.count(textureAbsolutePath)) {
            return textureIdx[textureAbsolutePath];
        }

        int width, height, channels;
        unsigned char *imgData;
        bool alpha = false;
        if (const aiTexture *texture = scene->GetEmbeddedTexture(path.C_Str())) {
            if (texture->mHeight == 0) { // Compressed image format
                imgData = stbi_load_from_memory((unsigned char *)texture->pcData, texture->mWidth, &width, &height, &channels, 0);
            } else {
                std::cout << "Warning: this case of texture is not properly handled\n";
                // I didn't add all the logic needed to handle this case, e.g. RGBA not handled, 1-channel images not handled, etc.
                imgData = reinterpret_cast<unsigned char*>(texture->pcData);
            }
        } else {
            // If assimp failed to load it, load it the classic way
            imgData = stbi_load(textureAbsolutePath.c_str(), &width, &height, &channels, 0);
        }

        // No image
        if (imgData == nullptr) {
            std::cout << "Error in loading the texture image\n" << std::endl;
            return -1;
        }

        // Texture expects a vector, not an array
        std::vector<char> dataVector(imgData, imgData + width * height * channels);

        // Switch to our type of texture enum
        enum TexType sgeType;
        switch (type) {
            case aiTextureType_DIFFUSE:
                sgeType = DIFFUSE_TEXTURE;
                break;
            case aiTextureType_HEIGHT:
                sgeType = BUMP_MAP;
                break;
            case aiTextureType_SHININESS:
                sgeType = SHININESS_TEXTURE;
                break;
            default:
                sgeType = UNKNOWN_TEXTYPE;
                break;
        }

        // Add texture to sge data structures
        textureIdx[textureAbsolutePath] = textures.size();
        textures.push_back(Texture(width, height, channels, sgeType, dataVector));

        // Feed texture to OpenGL
        glActiveTexture(GL_TEXTURE0 + sgeType);
        texID.push_back(0);
        glGenTextures(1, &texID.back());
        glBindTexture(GL_TEXTURE_2D, texID.back());
        // Handle different number of channels in texture
        int format = GL_RGB;
        if (channels == 1) {
            format = GL_RED;
        } else if (channels == 4) {
            format = GL_RGBA;
        }

        // Initialize texture settings within OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, dataVector.data());
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        stbi_image_free(imgData);
        glBindTexture(GL_TEXTURE_2D, 0);
        return textureIdx[textureAbsolutePath];
    }

    /**
     * Mesh constructor
     * @param NumIndices Number of indices in object - 3 * number of faces
     * @param BaseVertex Starting index in ModelComposite vertex array of current mesh
     * @param BaseIndex Starting index in ModelComposite indices array of current mesh
     * @param MaterialIndex Index in ModelComposite's material array - each mesh has one material
     */
    Mesh::Mesh(unsigned int NumIndices, unsigned int BaseVertex,
               unsigned BaseIndex, unsigned int MaterialIndex) : NumIndices(NumIndices), BaseVertex(BaseVertex), BaseIndex(BaseIndex), MaterialIndex(MaterialIndex) {}

   /**
    * Create a material object without diffuse texture map
    * @param specular
    * @param emissive
    * @param ambient
    * @param diffuse
    */
    Material::Material(glm::vec3 specular, glm::vec3 emissive, glm::vec3 ambient, glm::vec3 diffuse) : specular(specular), emissive(emissive), ambient(ambient), diffuse(diffuse), diffuseMap(-1) {}

    /**
     * Create a material object with diffuse texture map
     * TODO: be able to handle other texture maps and being able to fall back onto material properties if no texture exists
     * @param specular
     * @param emissive
     * @param ambient
     * @param diffuse
     * @param diffuseMap
     */
    Material::Material(glm::vec3 specular, glm::vec3 emissive, glm::vec3 ambient, glm::vec3 diffuse, int diffuseMap) : specular(specular), emissive(emissive), ambient(ambient), diffuse(diffuse), diffuseMap(diffuseMap) {}

    /**
     * Create a texture object
     * @param width Texture image width
     * @param height Texture image height
     * @param channels Number of channels within texture
     * @param data Image data
     */
    Texture::Texture(size_t width, size_t height, size_t channels, enum TexType type, std::vector<char> data)
            : width(width), height(height), channels(channels), type(type), data(data) {}

    std::unordered_map<std::string, int> textureIdx;
    std::vector<Texture> textures;
    std::vector<GLuint> texID;
}