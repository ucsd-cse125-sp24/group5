//
// Created by benjx on 4/7/2024.
//

#pragma once
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL // To be able to import gtx/quaternion.hpp (experimental glm feature)
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include "sge/GraphicsShaders.h"

#define ASSIMP_IMPORT_FLAGS aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_EmbedTextures | aiProcess_GenNormals | aiProcess_FixInfacingNormals | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_ValidateDataStructure | aiProcess_FindInstances | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes

// Maximum number of bones that may influence a single vertex
#define MAX_BONE_INFLUENCE 4
// Maximum number of bones per model
#define MAX_BONES 100

/**
 * Vertex array object (VAO) organization
 */
enum VAOLayout {
    VERTEX_POS = 0,
    NORMAL_POS = 1,
    TEXCOORD_POS = 2,
    BONEIDX_POS = 3,
    BONEWEIGHT_POS = 4
};

/**
 * Indices of vertex buffer, normal buffer, etc. for ModelComposite's buffer array
 */
enum BufferIndex {
    VERTEX_BUF = 0,
    NORMAL_BUF = 1,
    TEXCOORD_BUF = 2,
    BONE_IDX = 3,
    BONE_WEIGHT = 4,
    INDEX_BUF = 5,
    NUM_BUFFERS = 6
};

/**
 * Indices of different models in models vector
 * (After they've been loaded)
 */
enum ModelIndex {
    MAP = 0,
    PLAYER_0 = 1, // Rename to summer player or something lat
    TEST_ANIMATION = 2,
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
        Texture(size_t width, size_t height, size_t channels, enum TexType type, std::vector<unsigned char> data);
        const size_t width;
        const size_t height;
        const size_t channels;
        const enum TexType type;
        std::vector<unsigned char> data;
    };

    /**
     * Material properties for a mesh - determines how the mesh's surface will be colores/behave with lighting
     * e.g. Very shiny, very dark, etc.
     */
    class Material {
    public:
        Material(glm::vec3 specular,
                 glm::vec3 emissive,
                 glm::vec3 ambient,
                 glm::vec3 diffuse,
                 glm::vec3 shininess);
        Material(glm::vec3 specular,
                 glm::vec3 emissive,
                 glm::vec3 ambient,
                 glm::vec3 diffuse,
                 glm::vec3 shininess,
                 int diffuseMap,
                 int specularMap,
                 int bumpMap,
                 int displacementMap,
                 int roughMap);
        const glm::vec3 specular;
        const glm::vec3 emissive;
        const glm::vec3 ambient;
        const glm::vec3 diffuse;
        const glm::vec3 shininess;
        // Texture indices
        const int diffuseMap;
        const int specularMap;
        const int bumpMap;
        const int displacementMap;
        const int roughMap;
//        const int bumpMap;
//        const int normalMap;
//        const int specularMap;
//        const int ambientOcclusion;
//        const int metal;
        void setShaderMaterial() const;
    };

    class ModelPose {

    };

    class BonePose {
    public:
        BonePose();
        BonePose(aiNodeAnim &channel, int id);
        glm::mat4 poseAtTime(float time);
        int boneId;
        std::vector<glm::vec3> positions;
        std::vector<float> positionTimestamps;

        std::vector<glm::quat> rotations;
        std::vector<float> rotationTimestamps;

        std::vector<glm::vec3> scales;
        std::vector<float> scaleTimestamps;

        glm::mat4 curTransform;
    private:
        glm::mat4 interpolatePosition(float time);
        glm::mat4 interpolateRotation(float time);
        glm::mat4 interpolateScale(float time);
    };

    class Animation {
    public:
        float duration;
        float ticksPerSecond;
        std::unordered_map<int, BonePose> channels;
        std::vector<glm::mat4> poseAtTime(float time);
    };

    /**
     * 3D model consisting of animated and non-animated components
     * Convention: That all loaded models have their center at (x, y, z) = (0, 0, 0)
     */
    class ModelComposite {
    public:
        ModelComposite(const std::string &filename);
        ~ModelComposite();

//        virtual void renderPose();
        virtual void render(const glm::vec3 &modelPosition, const float &modelYaw) const;
//        void render(glm::vec3 modelPosition, float modelYaw, float modelPitch, float modelRoll) const;
    private:
        /**
         * Hierarchy of bones
         */
        class BoneNode {
        public:
            int id; // Bone id
            std::vector<BoneNode> children; // Bone node children
        };
    protected:
        GLuint VAO = 0; // OpenGL Vertex Array Object
        GLuint buffers[NUM_BUFFERS] = {}; // OpenGL buffers for rendering model
        std::filesystem::path parentDirectory; // Directory .obj file is located in, used for loading textures/other assets
        std::vector<Mesh> meshes; // Vector of meshes that form the model
        std::vector<Material> materials; // Vector of materials used by individual meshes

        // Standard model properties
        std::vector<glm::vec3> vertices; // Vertex positions
        std::vector<glm::vec3> normals; // Surface normals
        std::vector<glm::vec2> texcoords; // Texture coordinates
        std::vector<GLuint> indices; // Vertex indices for primitive geometry

        // Animation properties
        // boneIdx and boneWeights are not 2d arrays or 2d vectors because it's easier to load them into OpenGL buffers this way
        struct {
            std::vector<GLint> indices; // Bone indices for each vertex - indicating which bones influence each vertex, size: MAX_BONE_INFLUENCE * num vertices
            std::vector<GLfloat> weights; // Amount each vertex is influenced by each bone (Should be in range [0, 1]), size MAX_BONE_INFLUENCE * num vertices
        } vertexBoneProperties;
        struct {
            std::vector<glm::mat4> inverseBindingMatrices; // Bone offset matrices, indexed by bone id's
            std::vector<glm::mat4> relativeTransforms; // Relative transformation from each bone to its parent, for bone hierarchy
            BoneNode root; // Root node in bone hierarchy
            std::unordered_map<std::string, unsigned int> boneMap;
        } bones;

        std::unordered_map<std::string, unsigned int> boneMap; // Auxiliary data structure when loading skeleton - maps Assimp bone names to integeres
        std::vector<Animation> animations;
        unsigned int numBones;
        bool animated;

        // Standard ModelComposite methods
        void loadMesh(aiMesh &mesh);
        int loadTexture(aiTextureType type, const aiScene *scene, const aiMaterial &material);
        void initBuffers();
        void reserveGeometrySpace(const aiScene *scene);
        void loadMaterials(const aiScene *scene);
        void updateBoneMatrices(int animation, float time);

        // Animation-related methods
        void loadMeshBones(aiMesh &mesh);
        void loadBone(aiBone &bone);
        BoneNode buildBoneHierarchy(aiNode *root);
        Animation loadAnimation(const aiAnimation &animation);
    };


    void updateCameraToFollowPlayer(glm::vec3 playerPosition, float yaw, float pitch);
    void deleteTextures();
    extern glm::vec3 cameraPosition, cameraDirection, cameraUp;
    extern glm::mat4 perspectiveMat;
    extern glm::mat4 viewMat;
    extern std::vector<std::unique_ptr<ModelComposite>> models;
    extern std::unordered_map<std::string, int> textureIdx; // Map to keep track of which textures have been loaded and their positions within textures vector
    extern std::vector<Texture> textures; // Vector of textures used by program, global vector so multiple models/meshes can use the same texture
    extern std::vector<GLuint> texID; // OpenGL texture identifiers
};
