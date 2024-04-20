//
// Created by benjx on 4/7/2024.
//

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include "sge/ShittyGraphicsEngine.h"

#define ASSIMP_IMPORT_FLAGS aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_EmbedTextures | aiProcess_GenNormals | aiProcess_FixInfacingNormals | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_ValidateDataStructure | aiProcess_FindInstances | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes

/**
 * Vertex array object (VAO) organization
 */
enum VAOLayout {
    VERTEX_POS = 0,
    NORMAL_POS = 1,
    TEXCOORD_POS = 2,
//    BONEIDX_POS = 3,
//    BONEWEIGHT_POS = 4
};

/**
 * Indices of vertex buffer, normal buffer, etc. for ModelComposite's buffer array
 */
enum BufferIndex {
    VERTEX_BUF = 0,
    NORMAL_BUF = 1,
    TEXCOORD_BUF = 2,
//    BONE_BUF = 3, // TODO: change this to 4 after we add bones n stuff
    INDEX_BUF = 3,
    NUM_BUFFERS = 4
};

/**
 * Texture types
 */
enum TexType {
    DIFFUSE_TEXTURE = 0,
    SPECULAR_TEXTURE = 1,
    BUMP_MAP = 2,
    DISPLACEMENT_MAP = 3,
    SHININESS_TEXTURE = 4,
    UNKNOWN_TEXTYPE = 5,
    NUM_TEXTURES = 6
};

/**
 * Indices of different models in models vector
 * (After they've been loaded)
 */
enum ModelIndex {
    MAP = 0,
    PLAYER_0 = 1, // Rename to summer player or something lat
//    UR_MOM = 2, // :)
    NUM_MODELS
};

/**
 * Shitty graphics engine (SGE)
 */
namespace sge {

    /**
     * Struct containing information for each mesh in ModelComposite
     */
    class Mesh {
    public:
        Mesh(unsigned int NumIndices, unsigned int BaseVertex, unsigned int BaseIndex, unsigned int MaterialIndex);
        const unsigned int NumIndices; // Number of indices in object - 3 * number of faces
        const unsigned int BaseVertex; // Starting index in ModelComposite vertex array of current mesh
        const unsigned int BaseIndex; // Starting index in ModelComposite indices array of current mesh
        const unsigned int MaterialIndex; // Index in ModelComposite's material array - each mesh has one material
    };

    class Texture {
    public:
        Texture(size_t width, size_t height, size_t channels, enum TexType type, std::vector<char> data);
        const size_t width;
        const size_t height;
        const size_t channels;
        const enum TexType type;
        std::vector<char> data;
    };

    /**
     * Material properties for a mesh - determines how the mesh's surface will be colores/behave with lighting
     * e.g. Very shiny, very dark, etc.
     */
    class Material {
    public:
        Material(glm::vec3 specular, glm::vec3 emissive, glm::vec3 ambient, glm::vec3 diffuse);
        Material(glm::vec3 specular, glm::vec3 emissive, glm::vec3 ambient, glm::vec3 diffuse, int diffuseMap);
        const glm::vec3 specular;
        const glm::vec3 emissive;
        const glm::vec3 ambient;
        const glm::vec3 diffuse;
        // Texture indices
        const int diffuseMap;
        int roughMap;
//        const int bumpMap;
//        const int normalMap;
//        const int specularMap;
//        const int ambientOcclusion;
//        const int metal;
    };

    /**
     * 3D model consisting of animated and non-animated components
     * Convention: That all loaded models have their center at (x, y, z) = (0, 0, 0)
     */
    class ModelComposite {
    public:
        ModelComposite(std::string filename);
        ~ModelComposite();

        static glm::vec3 cameraPosition, cameraDirection, cameraUp;
        static void updateCameraToFollowPlayer(glm::vec3 playerPosition, float yaw, float pitch);

        // TODO: change render to allow for instancing and animations
        void render(glm::vec3 modelPosition, float modelYaw) const;
//        void render(glm::vec3 modelPosition, float modelYaw, float modelPitch, float modelRoll) const;

    private:
        GLuint VAO = 0; // OpenGL Vertex Array Object
        GLuint buffers[NUM_BUFFERS] = {}; // OpenGL buffers for rendering model
        std::filesystem::path parentDirectory; // Directory .obj file is located in, used for loading textures/other assets
        std::vector<Mesh> meshes; // Vector of meshes that form the model
        std::vector<Material> materials; // Vector of materials used by individual meshes
        std::vector<glm::vec3> vertices; // Vertex positions
        std::vector<glm::vec3> normals; // Surface normals
        std::vector<glm::vec2> texcoords; // Texture coordinates
        std::vector<GLuint> indices; // Vertex indices for primitive geometry
        // TODO: add another vector for bones for animations n stuff
        void loadMesh(aiMesh &mesh);
        int loadTexture(aiTextureType type, const aiScene *scene, const aiMaterial &material);
        void loadMaterials(const aiScene *scene);
        void initBuffers();
        void reserveGeometrySpace(const aiScene *scene);
    };

    extern std::vector<std::unique_ptr<ModelComposite>> models;
    extern std::unordered_map<std::string, int> textureIdx; // Map to keep track of which textures have been loaded and their positions within textures vector
    extern std::vector<Texture> textures; // Vector of textures used by program
    extern std::vector<GLuint> texID; // OpenGL texture identifiers
};
