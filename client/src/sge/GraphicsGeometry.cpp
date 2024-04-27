//
// Created by benjx on 4/8/2024.
//

#include "sge/GraphicsGeometry.h"

#define STB_IMAGE_IMPLEMENTATION // Needed for stb_image.h
#define DISTANCE_BEHIND_PLAYER 3.0f
#define DISTANCE_ABOVE_PLAYER 2.0f

#include <stb_image.h>

/**
 * Shitty graphics engine (SGE)
 */
namespace sge {


    /**
     * PRECONDITION: OpenGL should already be initialized
     * Create a ModelComposite (A 3d object model composed of mesh(es))
     * @param filename Path to .obj file specifying ModelComposite
     */
    sge::ModelComposite::ModelComposite(std::string filename) {
        // Load model
        parentDirectory = std::filesystem::path(filename).remove_filename();
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(filename,
                                                 ASSIMP_IMPORT_FLAGS);
        if (scene == nullptr) {
            std::cerr << "Unable to load 3d model from path " << filename << std::endl;
            exit(EXIT_FAILURE);
        }
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

    void ModelComposite::render(glm::vec3 modelPosition, float modelYaw) const {
        glUseProgram(sge::program);
        glBindVertexArray(VAO);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), modelPosition); // This instance's transformation matrix - specifies instance's rotation, translation, etc.
        model = glm::rotate(model, glm::radians(modelYaw), glm::vec3(0.0f, -1.0f, 0.0f));
        // yaw (cursor movement) should rotate our player model AND the camera view, right? 
        // glm::mat4 modelview = glm::perspective(glm::radians(90.0f), (float)sge::windowWidth / (float)sge::windowHeight, 0.5f, 1000.0f) * glm::lookAt(glm::vec3(10, 10, -5), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0, 1, 0)) * model;
        glUniformMatrix4fv(sge::modelPos, 1, GL_FALSE, &model[0][0]);
        for (unsigned int i = 0; i < meshes.size(); i++) {
            const Material &mat = materials[meshes[i].MaterialIndex];
            if (mat.diffuseMap != -1) {
                // Tell shader there is a diffuse map
                glUniform1i(sge::hasDiffuseMap, 1);
                glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE);
                glBindTexture(GL_TEXTURE_2D, texID[mat.diffuseMap]);
                // todo: only need to redo texsampler stuff for different shader programs
            } else {
                // Tell shader there is no diffuse map
                glUniform1i(sge::hasDiffuseMap, 0);
                glUniform3fv(sge::diffuseColor, 1, &mat.diffuse[0]);
            }
            if (mat.specularMap != -1) {
                glUniform1i(sge::hasSpecularMap, 1);
                glActiveTexture(GL_TEXTURE0 + SPECULAR_TEXTURE);
                glBindTexture(GL_TEXTURE_2D, texID[mat.specularMap]);
            } else {
                glUniform1i(sge::hasSpecularMap, 0);
                glUniform3fv(sge::specularColor, 1, &mat.specular[0]);
            }

            if (mat.bumpMap != -1) {
                glUniform1i(sge::hasBumpMap, 1);
                glActiveTexture(GL_TEXTURE0 + BUMP_MAP);
                glBindTexture(GL_TEXTURE_2D, texID[mat.bumpMap]);
            } else {
                glUniform1i(sge::hasBumpMap, 0);
            }

            if (mat.displacementMap != -1) {
                glUniform1i(sge::hasDisplacementMap, 1);
                glActiveTexture(GL_TEXTURE0 + DISPLACEMENT_MAP);
                glBindTexture(GL_TEXTURE_2D, texID[mat.displacementMap]);
            } else {
                glUniform1i(sge::hasDisplacementMap, 0);
            }

            if (mat.roughMap != -1) {
                glUniform1i(sge::hasRoughMap, 1);
                glActiveTexture(GL_TEXTURE0 + SHININESS_TEXTURE);
                glBindTexture(GL_TEXTURE_2D, texID[mat.roughMap]);
            } else {
                glUniform1i(sge::hasRoughMap, 0);
                glUniform3fv(sge::roughColor, 1, &mat.shininess[0]);
            }

            glUniform3fv(sge::emissiveColor, 1, &mat.emissive[0]);
            glUniform3fv(sge::ambientColor, 1, &mat.ambient[0]);
//            glUniform3fv(sge)

            glDrawElementsBaseVertex(GL_TRIANGLES, meshes[i].NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * meshes[i].BaseIndex), meshes[i].BaseVertex);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glBindVertexArray(0);
    }

//    // TODO: fill in later
//    void ModelComposite::render(glm::vec3 modelPosition, float modelYaw, float modelPitch, float modelRoll) const {
//        throw std::logic_error("Render not implemented with yaw, pitch, roll parameters");
//    }

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
            glm::vec3 shininess(0.0f);

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

            float shininessTmp;
            aiReturn ret;
            ret = mat.Get(AI_MATKEY_SHININESS, shininessTmp);
            if (ret == AI_SUCCESS) {
                shininess.x = shininessTmp;
                shininess.y = shininessTmp; // This is intentional, putting them all to R
                shininess.z = shininessTmp;
            }
            if (ret != AI_SUCCESS || shininessTmp == 0) {
                specular.x = 0; // No shininess
                specular.y = 0;
                specular.z = 0;
            }

            int diffuseTexIdx = loadTexture(aiTextureType_DIFFUSE, scene, mat);
            int specularTexIdx = loadTexture(aiTextureType_SPECULAR, scene, mat);
            int bumpTexIdx = loadTexture(aiTextureType_HEIGHT, scene, mat); // get bump map
            int displacementTexIdx = loadTexture(aiTextureType_DISPLACEMENT, scene, mat);
            int roughTexIdx = loadTexture(aiTextureType_SHININESS, scene, mat); // get rough map
            materials.push_back(Material(specular,
                                         emissive,
                                         ambient,
                                         diffuse,
                                         shininess,
                                         diffuseTexIdx,
                                         specularTexIdx,
                                         bumpTexIdx,
                                         displacementTexIdx,
                                         roughTexIdx));
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
        std::vector<unsigned char> dataVector(imgData, imgData + width * height * channels);

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
    Material::Material(glm::vec3 specular,
                       glm::vec3 emissive,
                       glm::vec3 ambient,
                       glm::vec3 diffuse,
                       glm::vec3 shininess) :
                       specular(specular),
                       emissive(emissive),
                       ambient(ambient),
                       diffuse(diffuse),
                       shininess(shininess),
                       diffuseMap(-1),
                       specularMap(-1),
                       bumpMap(-1),
                       displacementMap(-1),
                       roughMap(-1){}

    /**
     * Create a material object with diffuse texture map
     * TODO: be able to handle other texture maps and being able to fall back onto material properties if no texture exists
     * @param specular
     * @param emissive
     * @param ambient
     * @param diffuse
     * @param diffuseMap
     */
    Material::Material(glm::vec3 specular,
                       glm::vec3 emissive,
                       glm::vec3 ambient,
                       glm::vec3 diffuse,
                       glm::vec3 shininess,
                       int diffuseMap,
                       int specularMap,
                       int bumpMap,
                       int displacementMap,
                       int roughMap) :
                       specular(specular),
                       emissive(emissive),
                       ambient(ambient),
                       diffuse(diffuse),
                       shininess(shininess),
                       diffuseMap(diffuseMap),
                       specularMap(specularMap),
                       bumpMap(bumpMap),
                       displacementMap(displacementMap),
                       roughMap(roughMap){}

    /**
     * Create a texture object
     * @param width Texture image width
     * @param height Texture image height
     * @param channels Number of channels within texture
     * @param data Image data
     */
    Texture::Texture(size_t width, size_t height, size_t channels, enum TexType type, std::vector<unsigned char> data)
            : width(width), height(height), channels(channels), type(type), data(data) {}

    glm::vec3 cameraPosition;
    glm::vec3 cameraDirection;
    glm::vec3 cameraUp;
    glm::mat4 perspectiveMat;
    glm::mat4 viewMat;

    /**
     * Updates camera lookat matrix - the lookat matrix transforms vertices from world coordinates to camera coordinates
     * @param playerPosition Player position
     * @param yaw Camera yaw
     * @param pitch Camera pitch
     */
    void updateCameraToFollowPlayer(glm::vec3 playerPosition, float yaw, float pitch) {
        // the camera and the player should face the same direction (?)
        cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraDirection.y = sin(glm::radians(pitch));
        cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        // the camera is D distance behind the player
        cameraPosition = playerPosition - (cameraDirection * DISTANCE_BEHIND_PLAYER);

        // Send camera position to shaders
        glUniform3fv(sge::cameraPositionPos, 1, &cameraPosition[0]);
        // update camera's up
        cameraUp = glm::cross(glm::cross(cameraDirection, glm::vec3(0, 1, 0)), cameraDirection);

        viewMat = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);
        glUniformMatrix4fv(sge::viewPos, 1, GL_FALSE, &viewMat[0][0]);
    }

    // For some reason it only works if it's unique pointers, i don't know why
    std::vector<std::unique_ptr<ModelComposite>> models;
    std::unordered_map<std::string, int> textureIdx;
    std::vector<Texture> textures;
    std::vector<GLuint> texID;
}