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

#define ASSIMP_IMPORT_FLAGS aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_EmbedTextures | aiProcess_SortByPType | aiProcess_ValidateDataStructure | aiProcess_FindInstances | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes

/**
 * Shitty graphics engine (SGE)
 */
namespace sge {
    /**
     * Vertex array object layout
     */
    enum {
        POSITION_VB, // vec3
        NORMAl_VB, // vec3
        TEXCOORD_VB, // vec2
        BONE_VB, //
        NUM_BUFFERS,
    };

    class Renderable {

    };

    class Mesh {
    public:
        Mesh(unsigned int NumIndices, unsigned int BaseVertex, unsigned int BaseIndex, unsigned int MaterialIndex);
        const unsigned int NumIndices;
        const unsigned int BaseVertex;
        const unsigned int BaseIndex;
        const unsigned int MaterialIndex;
    };
    enum {
        NUM_MESHES
    };

    /**
     * 3D model consisting of animated and non-animated components
     */
    class ModelComposite {
    public:
        ModelComposite(std::string filename);
        ~ModelComposite();
    private:
        std::vector<Mesh> meshes;
        std::vector<int> m;
        GLuint VAO, VBO, TBO, EBO;
        std::vector<glm::vec3> vertices; // Vertex positions
        std::vector<glm::vec3> normals; // Surface normals
        std::vector<glm::vec2> texcoords; // Texture coordinates
        std::vector<GLuint> indices; // Vertex indices for primitive geometry
        // TODO: add another vector for bones for animations n stuff
        void loadMesh(aiMesh &mesh);
        void reserveGeometrySpace(const aiScene *scene);
    };
};

#endif //GROUP5_GRAPHICSGEOMETRY_H
