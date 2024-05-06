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
#include <functional>

#include "sge/GraphicsConstants.h"
#include "sge/GraphicsShaders.h"

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

    typedef std::vector<glm::mat4> ModelPose; // Final transformation matrices for each bone (hierarchy and stuff accounted for)

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
        virtual void renderPose(const glm::vec3 &modelPosition, const float &modelYaw, std::vector<glm::mat4> pose) const;
//        void render(glm::vec3 modelPosition, float modelYaw, float modelPitch, float modelRoll) const;
        ModelPose animationPose(int animationId, float time);
    private:
        /**
         * Hierarchy of bones
         */
        class BoneNode {
        public:
            int id; // Bone id
            glm::mat4 relativeTransform;
            std::vector<BoneNode> children; // Bone node children
        };

        class BonePose {
        public:
            BonePose();
            BonePose(aiNodeAnim &channel, int id);
            glm::mat4 poseAtTime(double time);
            int boneId;
            std::vector<glm::vec3> positions;
            std::vector<double> positionTimestamps;

            std::vector<glm::quat> rotations;
            std::vector<double> rotationTimestamps;

            std::vector<glm::vec3> scales;
            std::vector<double> scaleTimestamps;
        private:
            glm::mat4 interpolatePosition(double time);
            glm::mat4 interpolateRotation(double time);
            glm::mat4 interpolateScale(double time);
        };

        class Animation {
        public:
            double duration;
            double ticksPerSecond;
            std::unordered_map<int, BonePose> channels; // Maps bone indices to boneposes
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
            std::vector<int> indices; // Bone indices for each vertex - indicating which bones influence each vertex, size: MAX_BONE_INFLUENCE * num vertices
            std::vector<GLfloat> weights; // Amount each vertex is influenced by each bone (Should be in range [0, 1]), size MAX_BONE_INFLUENCE * num vertices
        } boneVertexWeights;
        struct {
            // Inverse binding matrices is not in the bonenode struct because not every node has one
            std::vector<glm::mat4> inverseBindingMatrices; // Bone offset matrices, maps from binding coordinates to bone/joint coordinates, array indexed by bone id's
            std::unordered_map<std::string, unsigned int> boneMap;
            BoneNode root; // Root node in bone hierarchy
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

        // Animation-related methods
        void loadMeshBones(aiMesh &mesh);
        void loadBone(aiBone &bone);
        BoneNode buildBoneHierarchy(aiNode *root);
        Animation loadAnimation(const aiAnimation &animation);

        void recursePose(ModelPose &out, Animation &anim, float time, glm::mat4 accumulator, BoneNode cur);
    };


    void updateCameraToFollowPlayer(glm::vec3 playerPosition, float yaw, float pitch);
    void deleteTextures();
    extern glm::vec3 cameraPosition, cameraDirection, cameraUp;
    extern glm::mat4 perspectiveMat;
    extern glm::mat4 viewMat;
    extern std::vector<std::unique_ptr<ModelComposite>> models;
    extern std::unordered_map<std::string, size_t> textureIdx; // Map to keep track of which textures have been loaded and their positions within textures vector
    extern std::vector<Texture> textures; // Vector of textures used by program, global vector so multiple models/meshes can use the same texture
    extern std::vector<GLuint> texID; // OpenGL texture identifiers
};
