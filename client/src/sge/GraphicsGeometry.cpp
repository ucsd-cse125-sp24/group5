//
// Created by benjx on 4/8/2024.
//

#include "sge/GraphicsGeometry.h"

/**
 * Shitty graphics engine (SGE)
 */
namespace sge {
    /**
     * Create a ModelComposite (A 3d object model composed of mesh(es))
     * @param filename Path to .obj file specifying ModelComposite
     */
    sge::ModelComposite::ModelComposite(std::string filename) {
        // Load model
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(filename,
                                                 ASSIMP_IMPORT_FLAGS);
        // Allocate space for meshes, vertices, normals, etc.
        meshes.reserve(scene->mNumMeshes);
        reserveGeometrySpace(scene);

        // Load meshes into ModelComposite data structures
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            loadMesh(*scene->mMeshes[i]);
        }
        // TODO: load materials
        initBuffers();
        importer.FreeScene();
    }

    sge::ModelComposite::~ModelComposite() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(NUM_BUFFERS, buffers);
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
        // TODO: allocate more buffers for bones if necessary

        glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTEX_BUF]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(VERTEX_POS);
        glVertexAttribPointer(VERTEX_POS, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[NORMAL_BUF]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * vertices.size(), &normals[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(NORMAL_POS);
        glVertexAttribPointer(VERTEX_POS, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[TEXCOORD_BUF]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords[0]) * texcoords.size(), &texcoords[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(TEXCOORD_POS);
        glVertexAttribPointer(VERTEX_POS, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        // TODO: allocate space for bones n stuff later

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_BUF]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    /**
     * Allocate enough space for all vertices, normals, texture coordinates, etc for ModelComposite
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

    void ModelComposite::render() {
        glBindVertexArray(VAO);

        glm::mat4 modelview = glm::perspective(glm::radians(60.0f), (float)sge::windowWidth / (float)sge::windowHeight, 10.0f, 100.0f) * glm::lookAt(glm::vec3(10, 0, 0), glm::vec3(0), glm::vec3(0, 1, 0));
        for (unsigned int i = 0; i < meshes.size(); i++) {
            glDrawElementsBaseVertex(GL_TRIANGLES, meshes[i].NumIndices, GL_UNSIGNED_INT, (void *)&indices[meshes[i].BaseIndex], meshes[i].BaseVertex);
        }

        glBindVertexArray(0);
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

}