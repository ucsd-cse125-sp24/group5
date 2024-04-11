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
#include "sge/ShittyGraphicsEngine.h"

#define ASSIMP_IMPORT_FLAGS aiProcess_Triangulate | aiProcess_GenNormals |aiProcess_FixInfacingNormals | aiProcess_JoinIdenticalVertices | aiProcess_EmbedTextures | aiProcess_SortByPType | aiProcess_ValidateDataStructure | aiProcess_FindInstances | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes

#define VERTEX_POS 0
#define NORMAL_POS 1
#define TEXCOORD_POS 2
#define BONEIDX_POS 3
#define BONEWEIGHT_POS 4

#define VERTEX_BUF 0
#define NORMAL_BUF 1
#define TEXCOORD_BUF 2
#define BONE_BUF 3
#define INDEX_BUF 4
#define NUM_BUFFERS 3

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

    /**
     * Material properties for a mesh - determines how the mesh's surface will be colores/behave with lighting
     * e.g. Very shiny, very dark, etc.
     */
    class Material {
    public:
        Material(float specular, float shininess, float emission, float ambient, float diffuse);
        const float specular;
        const float shininess;
        const float emission;
        const float ambient;
        const float diffuse;
    };

    /**
     * 3D model consisting of animated and non-animated components
     */
    class ModelComposite {
    public:
        ModelComposite(std::string filename);
        ~ModelComposite();

        void render();

    private:
        std::vector<Mesh> meshes;
        std::vector<Material> materials;
        GLuint VAO;
        GLuint buffers[NUM_BUFFERS];
        std::vector<glm::vec3> vertices; // Vertex positions
        std::vector<glm::vec3> normals; // Surface normals
        std::vector<glm::vec2> texcoords; // Texture coordinates
        std::vector<GLuint> indices; // Vertex indices for primitive geometry
        // TODO: add another vector for bones for animations n stuff
        void loadMesh(aiMesh &mesh);
        void loadMaterials(const aiScene *scene);
        void initBuffers();
        void reserveGeometrySpace(const aiScene *scene);
    };
};

#endif //GROUP5_GRAPHICSGEOMETRY_H
