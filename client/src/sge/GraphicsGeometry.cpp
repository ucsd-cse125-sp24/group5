//
// Created by benjx on 4/8/2024.
//

#include "sge/GraphicsGeometry.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

    void ModelComposite::render() const {
        glUseProgram(sge::program);
        glBindVertexArray(VAO);
        glm::mat4 modelview = glm::perspective(glm::radians(90.0f), (float)sge::windowWidth / (float)sge::windowHeight, 0.5f, 100.0f) * glm::lookAt(glm::vec3(10, 5, 5), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(sge::modelViewPos, 1, GL_FALSE, &modelview[0][0]);
        for (unsigned int i = 0; i < meshes.size(); i++) {
            glDrawElementsBaseVertex(GL_TRIANGLES, meshes[i].NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * meshes[i].BaseIndex), meshes[i].BaseVertex);
        }

        glBindVertexArray(0);
    }

    void ModelComposite::loadMaterials(const aiScene *scene) {
        std::cout<< scene->mNumTextures << std::endl;
        std::unordered_map<std::string, int> texture_idx; // maps textures to indices
        for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
            const aiMaterial &mat = *scene->mMaterials[i];
            aiColor4D color(0.f, 0.f, 0.f, 0.0f);
            int shadingModel = 0;

            glm::vec3 diffuse(0.0f);
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
            // Load textures
            aiString path;
            // TODO: add FreeImage or something to repo and load the texture in :)
            // use unordered map to keep track of which textures have been loaded and their indices in a texture array
            // have individual materials index into the texture array to
            for (unsigned int i = 0; i < 22; i++) {
                if (mat.GetTexture(static_cast<aiTextureType>(i), 0, &path) == AI_SUCCESS) {
                    std::string textureRelativePath(path.C_Str());
                    // Prevent duplicate loads of the same texture
                    if (texture_idx.count(textureRelativePath)) {
                        continue;
                    }
                    loadTexture(parentDirectory.string() + textureRelativePath);
                    texture_idx[std::string(path.C_Str())] = textures.size();
                    std::cout << i << std::endl;
                }
            }
            materials.push_back(Material(specular, emissive, ambient, diffuse));
        }
    }

    void ModelComposite::loadTexture(std::string texturePath) {
        std::cout << "Loading texture from " << texturePath << std::endl;

        int width, height, channels;

        // stbi_load expects a const char*, not a std::string
        const char* filePath = texturePath.c_str();

        unsigned char* dataPtr = stbi_load(filePath, &width, &height, &channels, 0);
        if (dataPtr == NULL) {
            std::cout << "Error in loading the texture image\n" << std::endl;
            return;
        }

        // Texture expects a vector, not an array
        std::vector<char> dataVector(dataPtr, dataPtr + width*height*channels);

        Texture tex(width, height, channels, dataVector);

        textures.push_back(tex);
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

    Material::Material(glm::vec3 specular, glm::vec3 emissive, glm::vec3 ambient, glm::vec3 diffuse) : specular(specular), emissive(emissive), ambient(ambient), diffuse(diffuse) {}

    Texture::Texture(size_t width, size_t height, size_t channels, std::vector<char> data) : width(width), height(height), channels(channels), data(data) {}
}