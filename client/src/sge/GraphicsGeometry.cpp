//
// Created by benjx on 4/8/2024.
//

#include "sge/GraphicsGeometry.h"

/**
 * Shitty graphics engine (SGE)
 */
namespace sge {
    sge::ModelComposite::ModelComposite(std::string filename) {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile("./model/test/tank/Tiger_I.obj",
                                                 ASSIMP_IMPORT_FLAGS);
        meshes.reserve(scene->mNumMeshes);
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        reserveGeometrySpace(scene);
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            loadMesh(*scene->mMeshes[i]);
        }
        // TODO: load materials
        // TODO: allocate buffers
        glBindVertexArray(0);
        importer.FreeScene();
    }

    sge::ModelComposite::~ModelComposite() {
        glDeleteVertexArrays(1, &VAO);
    }

    void ModelComposite::loadMesh(aiMesh &mesh) {
        assert(mesh.mNormals != nullptr);
        assert(mesh.mVertices != nullptr);

        for (unsigned int i = 0; i < mesh.mNumVertices; i++) {
            const aiVector3D &vertex = mesh.mVertices[i];
            vertices.push_back(glm::vec3(vertex[0], vertex[1], vertex[2]));
            const aiVector3D &normal = mesh.mNormals[i];
            normals.push_back(glm::vec3(normal[0], normal[1], normal[2]));
            if (mesh.HasTextureCoords(0)) {
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
     * Allocate enough space for all vertices, normals, texture coordinates, etc for composite
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
        if (scene->HasTextures()) {
            texcoords.reserve(num_vertex);
        }
        if (scene->HasAnimations()) {
            // TODO: fill in
            // bones.reserve(num_indices); or something idk
        }
    }

    Mesh::Mesh(unsigned int NumIndices, unsigned int BaseVertex,
               unsigned BaseIndex, unsigned int MaterialIndex) : NumIndices(NumIndices), BaseVertex(BaseVertex), BaseIndex(BaseIndex), MaterialIndex(MaterialIndex) {}

}