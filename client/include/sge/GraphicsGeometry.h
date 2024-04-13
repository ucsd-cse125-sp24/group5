//
// Created by benjx on 4/7/2024.
//

#ifndef GROUP5_GRAPHICSGEOMETRY_H
#define GROUP5_GRAPHICSGEOMETRY_H
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

#define ASSIMP_IMPORT_FLAGS aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_EmbedTextures| aiProcess_GenNormals |aiProcess_FixInfacingNormals | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_ValidateDataStructure | aiProcess_FindInstances | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes

#define VERTEX_POS 0
#define NORMAL_POS 1
#define TEXCOORD_POS 2
#define BONEIDX_POS 3
#define BONEWEIGHT_POS 4

#define VERTEX_BUF 0
#define NORMAL_BUF 1
#define TEXCOORD_BUF 2
#define BONE_BUF 3
#define INDEX_BUF 3 // TODO: change this to 4 after we add bones n stuff
#define NUM_BUFFERS 4 // TODO: don't fuck this line up again

#define DIFFUSE_TEXTURE 0
#define SPECULAR_TEXTURE 1
#define BUMP_MAP 2
#define DISPLACEMENT_MAP 3
#define NUM_TEXURES 4


/**
 * Shitty graphics engine (SGE)
 */
namespace sge {

    class Renderable {

    };

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
        Texture(size_t width, size_t height, size_t channels, std::vector<char> data);
        const size_t width;
        const size_t height;
        const size_t channels;
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
        std::vector<bool> has_tex;
        // Texture indices
        const int diffuseMap;
//        const int roughMap;
//        const int bumpMap;
//        const int normalMap;
//        const int specularMap;
//        const int ambientOcclusion;
//        const int metal;
    };

    /**
     * 3D model consisting of animated and non-animated components
     */
    class ModelComposite {
    public:
        ModelComposite(std::string filename);
        ~ModelComposite();

        // TODO: change render to allow for instancing and animations
        void render() const;

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

    extern std::unordered_map<std::string, int> textureIdx; // Map to keep track of which textures have been loaded and their positions within textures vector
    extern std::vector<Texture> textures; // Vector of textures used by program
    extern std::vector<GLuint> texID; // OpenGL texture identifiers
};


#endif //GROUP5_GRAPHICSGEOMETRY_H
