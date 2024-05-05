//
// Created by benjx on 4/8/2024.
//

#include "sge/GraphicsGeometry.h"

#define STB_IMAGE_IMPLEMENTATION // Needed for stb_image.h
#define DISTANCE_BEHIND_PLAYER 3.0f
#define DISTANCE_ABOVE_PLAYER 2.0f

#include <stb_image.h>

/**
 * Converts an assimp matrix (row-major) to a GLM matrix (column-major)
 * @param mat
 * @return
 */
static glm::mat4 assimpToGlmMat4(aiMatrix4x4 mat) {
    return glm::mat4(mat[0][0], mat[1][0], mat[2][0], mat[3][0],
                     mat[0][1], mat[1][1], mat[2][1], mat[3][1],
                     mat[0][2], mat[1][2], mat[2][2], mat[3][2],
                     mat[0][3], mat[1][3], mat[2][3], mat[3][3]);
}

/**
 * Shitty graphics engine (SGE)
 */
namespace sge {


    /**
     * PRECONDITION: OpenGL should already be initialized
     * Create a ModelComposite (A 3d object model composed of mesh(es))
     * @param filename Path to .obj file specifying ModelComposite
     */
    sge::ModelComposite::ModelComposite(const std::string &filename) {
        // Load model
        parentDirectory = std::filesystem::path(filename).remove_filename();
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(filename,
                                                 ASSIMP_IMPORT_FLAGS);
        if (scene == nullptr) {
            std::cerr << "Unable to load 3d model from path " << filename << std::endl;
            exit(EXIT_FAILURE);
        }

        animated = scene->HasAnimations();

        // Allocate space for meshes, vertices, normals, etc.
        meshes.reserve(scene->mNumMeshes);
        materials.reserve(scene->mNumMaterials);

        // Allocate space for
        reserveGeometrySpace(scene);

        // Load meshes into ModelComposite data structures
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            loadMesh(*scene->mMeshes[i]);
        }

        if (animated) {
            // Loading bones handled by loadMesh (loadMesh does not load bone hierarchy)

            for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
                animations.push_back(loadAnimation(*scene->mAnimations[i]));
            }

            // Load bone hierarchy
            bones.root = buildBoneHierarchy(scene->mRootNode);
        }
        loadMaterials(scene);
        initBuffers();
        importer.FreeScene();
    }

    /**
     * Destructor
     */
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

        // Load faces/triangles into indices
        for (unsigned int i = 0; i < mesh.mNumFaces; i++) {
            const aiFace &face = mesh.mFaces[i];
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }

        if (animated) {
            loadMeshBones(mesh);
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

        // Create VAO stuff for animation-related vertex properties
        if (animated) {
            glBindBuffer(GL_ARRAY_BUFFER, buffers[BONE_IDX]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(boneVertexWeights.indices[0]) * boneVertexWeights.indices.size(), &boneVertexWeights.indices[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(BONEIDX_POS);
            glVertexAttribPointer(BONEIDX_POS, MAX_BONE_INFLUENCE, GL_INT, GL_FALSE, 0, nullptr);

            glBindBuffer(GL_ARRAY_BUFFER, buffers[BONEWEIGHT_POS]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(boneVertexWeights.weights[0]) * boneVertexWeights.weights.size(), &boneVertexWeights.weights[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(BONEWEIGHT_POS);
            glVertexAttribPointer(BONEWEIGHT_POS, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, 0, nullptr);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_BUF]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    /**
     * Allocate enough space for all vertices, normals, texture coordinates, etc for ModelComposite
     * and add individual meshes to "meshes" vector
     *
     * This function only exists to avoid having our vectors continually reallocate space
     * when loading larger models
     *
     * PRECONDITION: this->animated has already been set
     */
    void ModelComposite::reserveGeometrySpace(const aiScene *scene) {
        assert(scene != nullptr);
        unsigned int num_vertex = 0;
        unsigned int num_indices = 0;
        unsigned int num_bones = 0;
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            const aiMesh &mesh = *scene->mMeshes[i];
            // NumIndices = 3 * NumFaces as each face is a triangle, which consists of 3 vertices. Each index references a specific vertex
            meshes.push_back(Mesh(mesh.mNumFaces * 3, num_vertex, num_indices, mesh.mMaterialIndex));
            num_vertex += mesh.mNumVertices;
            num_indices += meshes.back().NumIndices;
            num_bones += mesh.mNumBones;
        }
        vertices.reserve(num_vertex);
        normals.reserve(num_vertex);
        indices.reserve(num_indices);
        texcoords.reserve(num_vertex);

        if (animated) {
            // vector.assign automatically allocates space in the vector
            // All bone indices initialized to -1 and boneweights initialized to 0 by default in reserveGeometrySpace
            // Directly initializing stuff here because bone properties may not be loaded in sequential order
            boneVertexWeights.indices.assign(MAX_BONE_INFLUENCE * num_vertex, -1);
            boneVertexWeights.weights.assign(MAX_BONE_INFLUENCE * num_vertex, 0);
            bones.inverseBindingMatrices.reserve(num_bones);
            numBones = num_bones;
        } else {
            numBones = 0;
        }
    }

    /**
     * Render the model with a static pose
     * @param modelPosition Model position in world coordinates
     * @param modelYaw Model yaw in degrees
     */
    void ModelComposite::render(const glm::vec3 &modelPosition, const float &modelYaw) const {
        defaultProgram.useShader();
        glBindVertexArray(VAO);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), modelPosition); // This instance's transformation matrix - specifies instance's rotation, translation, etc.
        model = glm::rotate(model, glm::radians(modelYaw), glm::vec3(0.0f, -1.0f, 0.0f));
        // yaw (cursor movement) should rotate our player model AND the camera view, right?
        defaultProgram.updateModelMat(model);
        // Draw each mesh to the screen
        for (unsigned int i = 0; i < meshes.size(); i++) {
            const Material &mat = materials[meshes[i].MaterialIndex];
            mat.setShaderMaterial();
            glDrawElementsBaseVertex(GL_TRIANGLES, meshes[i].NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * meshes[i].BaseIndex), meshes[i].BaseVertex);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glBindVertexArray(0);
    }


    /**
     * Loads all material properties from a scene into ModelComposite material vector
     * @param scene Scene to load materials from
     */
    void ModelComposite::loadMaterials(const aiScene *scene) {
        for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
            const aiMaterial &mat = *scene->mMaterials[i];
            aiColor4D color(0.f, 0.f, 0.f, 0.0f);

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


            int diffuseTexIdx = loadTexture(aiTextureType_DIFFUSE, scene, mat);
            int specularTexIdx = loadTexture(aiTextureType_SPECULAR, scene, mat);
            int bumpTexIdx = loadTexture(aiTextureType_HEIGHT, scene, mat); // get bump map
            int displacementTexIdx = loadTexture(aiTextureType_DISPLACEMENT, scene, mat);
            int roughTexIdx = loadTexture(aiTextureType_SHININESS, scene, mat); // get rough map

            if (ret != AI_SUCCESS || (shininessTmp == 0 && roughTexIdx == -1)) {
                specular.x = 0; // No shininess
                specular.y = 0;
                specular.z = 0;
            }

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
            case aiTextureType_SPECULAR:
                sgeType = SPECULAR_TEXTURE;
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
     * Load an animation from an assimp scene object and add it to
     * ModelComposite data structures
     * @param animation
     * @return
     */
    ModelComposite::Animation ModelComposite::loadAnimation(const aiAnimation &animation) {
        Animation cur;
        cur.duration = animation.mDuration;
        cur.ticksPerSecond = animation.mTicksPerSecond;
        for (unsigned int i = 0; i < animation.mNumChannels; i++) {
            std::string nodeName = animation.mChannels[i]->mNodeName.C_Str();
            if (!boneMap.count(nodeName)) {
                boneMap[nodeName] = boneMap.size();
                bones.inverseBindingMatrices.push_back(glm::mat4(1)); // To maintain invariant that this vector can always be indexed by bone id, tho this matrix (should) never be used
            }
            int id = boneMap[nodeName];
            cur.channels[id] = BonePose(*animation.mChannels[i], id);
        }
        return cur;
    }

    /**
     * Load mesh skeletons and bones
     * @param mesh
     */
    void ModelComposite::loadMeshBones(aiMesh &mesh) {
        for (unsigned int i = 0; i < mesh.mNumBones; i++) {
            loadBone(*mesh.mBones[i]);
        }
    }

    /**
     * Load an individual bone (for the first time)
     * Bone should not be loaded again or spicy things will happen
     * @param bone
     */
    void ModelComposite::loadBone(aiBone &bone) {
        std::string boneName = bone.mName.C_Str();
        int boneIdx;
        glm::mat4 mat = assimpToGlmMat4(bone.mOffsetMatrix);
        if (boneMap.count(boneName)) {
            std::cout << "Warning: Overwriting bone offset matrix" << std::endl;
            boneIdx = boneMap[boneName];
            bones.inverseBindingMatrices[boneIdx] = mat;
        } else {
            boneIdx = boneMap.size();
            boneMap[boneName] = boneIdx;
            bones.inverseBindingMatrices.push_back(mat);
        }

        // Load vertex weights
        for (unsigned int i = 0; i < bone.mNumWeights; i++) {
            aiVertexWeight &curweight = bone.mWeights[i];
            bool weightAdded = false;
            for (unsigned int j = 0; j < MAX_BONE_INFLUENCE; j++) {
                if (boneVertexWeights.indices[curweight.mVertexId * MAX_BONE_INFLUENCE + j] < 0) {
                    // TODO: if this breaks with multiple meshes, it might be because the vertexId's are relative to the current mesh,
                    // not the entire modelcomposite object (blame assimp)
                    boneVertexWeights.weights[curweight.mVertexId * MAX_BONE_INFLUENCE + j] = curweight.mWeight;
                    boneVertexWeights.indices[curweight.mVertexId * MAX_BONE_INFLUENCE + j] = boneIdx;
                    weightAdded = true;
                    break;
                }
            }
            if (!weightAdded) {
                std::cout << "Warning: Exceeded maximum number of bones per vertex, ignoring remaining weights" << std::endl;
            }
        }
    }

    /**
     * PRECONDITION: All meshes already loaded (and by extension, all bones directly attached to vertices have been loaded)
     *
     * Loads the model's bone hierarchy and load in any missing bones
     * @param root aiNode to build from
     * @return Resulting tree translated to ModelComposite::BoneNode's
     */
    ModelComposite::BoneNode ModelComposite::buildBoneHierarchy(aiNode *root) {
        BoneNode cur;
        std::string boneName = root->mName.C_Str();
        // Assimp is dumb and adds a bunch of extra nodes in the hierarchy tree with identity matrix relative transformations :/
        int boneId = -1;
        if (boneMap.count(boneName)) {
            boneId = boneMap[boneName];
        }
        cur.relativeTransform = assimpToGlmMat4(root->mTransformation);
        cur.id = boneId;
        for (unsigned int i = 0; i < root->mNumChildren; i++) {
            cur.children.push_back(buildBoneHierarchy(root->mChildren[i]));
            // Delete unnecessary nodes
            if (cur.children.back().id == -1 && cur.children.back().children.empty()) {
                cur.children.pop_back();
            }
        }

        return cur;
    }

    /**
     * Return model pose for a given animation at a given timestamp
     * TODO: change this to use an output parameter cus creating a new array every time is slow as fuck
     * @param animationId Model's animation identifier
     * @param time Timestamp to retrieve pose
     * @return Model's pose at given timestamp
     */
    ModelPose ModelComposite::animationPose(int animationId, float time) const {
        assert(animationId >= 0  && animationId < animations.size() && animated);
        ModelPose out(MAX_BONES, glm::mat4(1));
        Animation anim = animations[animationId];
        // Recursively construct final transformation matrices for each bone
        std::function<void (glm::mat4, BoneNode)> recursePose = [&](glm::mat4 accumulator, BoneNode cur) -> void {
            if (cur.id == -1) {
                accumulator = accumulator * cur.relativeTransform;
            } else {
                accumulator = accumulator * anim.channels[cur.id].poseAtTime(time);
                out[cur.id] = accumulator * bones.inverseBindingMatrices[cur.id];
            }
            for (const BoneNode &child : cur.children) {
                recursePose(accumulator, child);
            }
        };
        recursePose(glm::mat4(1), bones.root);
        return out;
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
    * Tell default shader about material properties to render
    */
    void Material::setShaderMaterial() const {
       if (diffuseMap != -1) {
           // Tell shader there is a diffuse map
           glUniform1i(defaultProgram.hasDiffuseMap, 1);
           glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE);
           glBindTexture(GL_TEXTURE_2D, texID[diffuseMap]);
       } else {
           // Tell shader there is no diffuse map
           glUniform1i(defaultProgram.hasDiffuseMap, 0);
           glUniform3fv(defaultProgram.diffuseColor, 1, &diffuse[0]);
       }
       if (specularMap != -1) {
           glUniform1i(defaultProgram.hasSpecularMap, 1);
           glActiveTexture(GL_TEXTURE0 + SPECULAR_TEXTURE);
           glBindTexture(GL_TEXTURE_2D, texID[specularMap]);
       } else {
           glUniform1i(defaultProgram.hasSpecularMap, 0);
           glUniform3fv(defaultProgram.specularColor, 1, &specular[0]);
       }

       if (bumpMap != -1) {
           glUniform1i(defaultProgram.hasBumpMap, 1);
           glActiveTexture(GL_TEXTURE0 + BUMP_MAP);
           glBindTexture(GL_TEXTURE_2D, texID[bumpMap]);
       } else {
           glUniform1i(defaultProgram.hasBumpMap, 0);
       }

       if (displacementMap != -1) {
           glUniform1i(defaultProgram.hasDisplacementMap, 1);
           glActiveTexture(GL_TEXTURE0 + DISPLACEMENT_MAP);
           glBindTexture(GL_TEXTURE_2D, texID[displacementMap]);
       } else {
           glUniform1i(defaultProgram.hasDisplacementMap, 0);
       }

       if (roughMap != -1) {
           glUniform1i(defaultProgram.hasRoughMap, 1);
           glActiveTexture(GL_TEXTURE0 + SHININESS_TEXTURE);
           glBindTexture(GL_TEXTURE_2D, texID[roughMap]);
       } else {
           glUniform1i(defaultProgram.hasRoughMap, 0);
           glUniform3fv(defaultProgram.roughColor, 1, &shininess[0]);
       }

       glUniform3fv(defaultProgram.emissiveColor, 1, &emissive[0]);
       glUniform3fv(defaultProgram.ambientColor, 1, &ambient[0]);
    }

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
     * WARNING: THIS WILL CHANGE THE ACTIVE SHADER PROGRAM
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
        defaultProgram.useShader();
        defaultProgram.updateCamPos(cameraPosition);
        cameraUp = glm::cross(glm::cross(cameraDirection, glm::vec3(0, 1, 0)), cameraDirection);
        viewMat = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);
        defaultProgram.updateViewMat(viewMat);

        screenProgram.useShader();
        screenProgram.updateCamPos(cameraPosition);
    }

    /**
     * Gracefully deallocate textures in OpenGL context
     */
    void deleteTextures() {
        if (texID.size() > 0) {
            glDeleteTextures(texID.size(), &texID[0]);
        }
    }

    // For some reason it only works if it's unique pointers, i don't know why
    std::vector<std::unique_ptr<ModelComposite>> models;
    std::unordered_map<std::string, size_t> textureIdx;
    std::vector<Texture> textures;
    std::vector<GLuint> texID;

    /**
     * Create a bonepose object from an Assimp animation channel
     * @param channel Assimp animation channel
     * @param id Bone id
     */
    ModelComposite::BonePose::BonePose(aiNodeAnim &channel, int id) {
        boneId = id;
        for (unsigned int i = 0; i < channel.mNumPositionKeys; i++) {
            aiVectorKey keyframePos = channel.mPositionKeys[i];
            positions.push_back(glm::vec3(keyframePos.mValue.x, keyframePos.mValue.y, keyframePos.mValue.z));
            positionTimestamps.push_back(keyframePos.mTime);
        }
        for (unsigned int i = 0; i < channel.mNumRotationKeys; i++) {
            aiQuatKey keyframeRot = channel.mRotationKeys[i];
            rotations.push_back(glm::quat(keyframeRot.mValue.w, keyframeRot.mValue.x, keyframeRot.mValue.y, keyframeRot.mValue.z));
            rotationTimestamps.push_back(keyframeRot.mTime);
        }
        for (unsigned int i = 0; i < channel.mNumScalingKeys; i++) {
            aiVectorKey keyframeSca = channel.mScalingKeys[i];
            scales.push_back(glm::vec3(keyframeSca.mValue.x, keyframeSca.mValue.y, keyframeSca.mValue.z));
            scaleTimestamps.push_back(keyframeSca.mTime);
        }
    }

    /**
     * Interpolate bone pose at time t
     * @param time
     * @return Transformation matrix for current bone relative to its parent joint
     */
    glm::mat4 ModelComposite::BonePose::poseAtTime(double time) {
        glm::mat4 translation = interpolatePosition(time);
        glm::mat4 rotation = interpolateRotation(time);
        glm::mat4 scale = interpolateScale(time);
        return translation * rotation * scale;
    }

    /**
     * Linearly interpolate between keyframes
     * @param time
     * @return
     */
    glm::mat4 ModelComposite::BonePose::interpolatePosition(double time) {
        // Default return when no keyframes available
        if (positions.empty()) return glm::mat4(1);

        int idx = std::lower_bound(positionTimestamps.begin(), positionTimestamps.end(), time) - positionTimestamps.begin();
        if (idx >= positionTimestamps.size() - 1) {
            return glm::translate(glm::mat4(1), positions.back());
        }
        glm::vec3 pos1 = positions[idx];
        glm::vec3 pos2 = positions[idx + 1];
        double t1 = positionTimestamps[idx];
        double t2 = positionTimestamps[idx + 1];
        float scalar = (time - t1) / (t2 - t1);
        return glm::translate(glm::mat4(1), glm::mix(pos1, pos2, scalar));
    }

    glm::mat4 ModelComposite::BonePose::interpolateRotation(double time) {
        // Default return when no keyframes available
        if (rotations.empty()) return glm::mat4(1);

        int idx = std::lower_bound(rotationTimestamps.begin(), rotationTimestamps.end(), time) - rotationTimestamps.begin();
        if (idx >= rotationTimestamps.size() - 1) {
            return glm::toMat4(rotations.back());
        }
        glm::quat rot1 = rotations[idx];
        glm::quat rot2 = rotations[idx + 1];
        double t1 = rotationTimestamps[idx];
        double t2 = rotationTimestamps[idx + 1];
        float scalar = (time - t1) / (t2 - t1);
        glm::quat finalRot = glm::normalize(glm::slerp(rot1, rot2, scalar));
        return glm::toMat4(finalRot);
    }

    // TODO: be able to handle empty scale vectors
    glm::mat4 ModelComposite::BonePose::interpolateScale(double time) {
        // Default return if no scale keyframes avaialble
        if (scales.empty()) return glm::mat4(1);

        int idx = std::lower_bound(scaleTimestamps.begin(), scaleTimestamps.end(), time) - scaleTimestamps.begin();
        if (idx >= scaleTimestamps.size() - 1) {
            return glm::scale(glm::mat4(1), scales.back());
        }
        glm::vec3 scale1 = scales[idx];
        glm::vec3 scale2 = scales[idx + 1];
        double t1 = scaleTimestamps[idx];
        double t2 = scaleTimestamps[idx + 1];
        float scalar = (time - t1) / (t2 - t1);
        return glm::scale(glm::mat4(1), glm::mix(scale1, scale2, scalar));
    }

    ModelComposite::BonePose::BonePose() {
        boneId = -1;
    }
}
