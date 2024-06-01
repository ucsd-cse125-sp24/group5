//
// Created by benjx on 4/8/2024.
//

#include "sge/GraphicsGeometry.h"

#define STB_IMAGE_IMPLEMENTATION // Needed for stb_image.h

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
        modelFilePath = filename;
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
            loadMesh(*scene->mMeshes[i], i);
        }

        if (animated) {
            // Loading bones handled by loadMesh (loadMesh does not load bone hierarchy)

            for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
                animations.push_back(loadAnimation(*scene->mAnimations[i]));
            }
            animationGlobalInverse = assimpToGlmMat4(scene->mRootNode->mTransformation.Inverse());
            // A bit of a hack since glb files are rotated 90 degrees
            animationGlobalInverse = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0, 1, 0)) * animationGlobalInverse;
            // Load bone hierarchy
            bones.root = buildBoneHierarchy(scene->mRootNode);

            // By default, use tick 0 of animation 0 for when no animation is happening
            animationWhenStill = 0;
            animationTickWhenStill = 0;
        }
        loadMaterials(scene);
        initBuffers();
        importer.FreeScene();
    }

    /**
     * Set animation to default "still" pose
     * @param animationId
     * @param animationTick
     */
    void sge::ModelComposite::setStillAnimation(unsigned int animationId, float animationTick) {
        assert(animationId >= 0 && animationId < animations.size() && animationTick >= 0 && animationTick < animations[animationId].duration && animated);
        animationWhenStill = animationId;
        animationTickWhenStill = animationTick;
    }

    /**
     * Modelcomposite destructor - Deletes VAO, VBO, etc. from OpenGL context
     */
    sge::ModelComposite::~ModelComposite() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(NUM_BUFFERS, buffers);
    }

    /**
     * Populate ModelComposite data structures with mesh data
     * @param mesh Mesh to load
     */
    void ModelComposite::loadMesh(aiMesh &mesh, int meshIdx) {
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
            for (unsigned int i = 0; i < mesh.mNumBones; i++) {
                loadBone(*mesh.mBones[i], meshIdx);
            }
        }
    }

    /**
     * Load an individual bone (for the first time)
     * Bone should not be loaded again or spicy things will happen
     * @param bone
     */
    void ModelComposite::loadBone(aiBone &bone, int meshIdx) {
        std::string boneName = bone.mName.C_Str();
        if (!boneMap.count(boneName)) {
            // Don't load the same bone twice
            boneMap[boneName] = boneMap.size();
            bones.offsetMatrices.push_back(assimpToGlmMat4(bone.mOffsetMatrix));
            numBones++;
        }

        int boneIdx = boneMap[boneName];

        // Load vertex weights
        for (unsigned int i = 0; i < bone.mNumWeights; i++) {
            aiVertexWeight &curweight = bone.mWeights[i];
            bool weightAdded = false;
            int offset = (meshes[meshIdx].BaseVertex + curweight.mVertexId) * MAX_BONE_INFLUENCE;
            if (curweight.mWeight == 0) continue;
            for (unsigned int j = 0; j < MAX_BONE_INFLUENCE; j++) {
                if (boneVertexWeights.indices[offset + j] < 0) {
                    // not the entire modelcomposite object (blame assimp)
                    boneVertexWeights.weights[offset + j] = curweight.mWeight;
                    boneVertexWeights.indices[offset + j] = boneIdx;
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
            glVertexAttribIPointer(BONEIDX_POS, MAX_BONE_INFLUENCE, GL_INT,  0, nullptr);

            glBindBuffer(GL_ARRAY_BUFFER, buffers[BONE_WEIGHT]);
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
            bones.offsetMatrices.reserve(num_bones);
            numBones = num_bones;
        } else {
            numBones = 0;
        }
    }

    /**
     * Render the model with a static pose
     * @param modelPosition Model position in world coordinates
     * @param modelYaw Model yaw in degrees
     * @param shadow Whether to render to shadowmap
     * @param outline Whether to render outline for current entity, does nothing if shadow is true
     */
    void ModelComposite::render(const glm::vec3 &modelPosition, const float &modelYaw, bool shadow, bool outline) const {
        if (shadow == true) {
            shadowProgram.useShader();
            shadowProgram.setAnimated(false);
        } else {
            defaultProgram.useShader();
            defaultProgram.setAnimated(false);
            defaultProgram.updateOutline(outline);
        }
        glBindVertexArray(VAO);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), modelPosition); // This instance's transformation matrix - specifies instance's rotation, translation, etc.
        model = glm::rotate(model, glm::radians(modelYaw), glm::vec3(0.0f, -1.0f, 0.0f));
        // yaw (cursor movement) should rotate our player model AND the camera view, right?
        if (shadow == true) {
            shadowProgram.updateModelMat(model);
        } else {
            defaultProgram.updateModelMat(model);
        }
        // Draw each mesh to the screen
        for (unsigned int i = 0; i < meshes.size(); i++) {
            if (shadow == false) {
                const Material &mat = materials[meshes[i].MaterialIndex];
                mat.setShaderMaterial();
            }
            glDrawElementsBaseVertex(GL_TRIANGLES, meshes[i].NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * meshes[i].BaseIndex), meshes[i].BaseVertex);
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

            glm::vec3 diffuse(-1.0f);
            glm::vec3 specular(0.0f);
            glm::vec3 shininess(0.0f);

            // We only allow blending between textures, not material colors
            // because .mtl files don't support many color types
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
            float shininessTmp;
            aiReturn ret;
            ret = mat.Get(AI_MATKEY_SHININESS, shininessTmp);
            if (ret == AI_SUCCESS) {
                shininess.x = shininessTmp;
                shininess.y = shininessTmp; // This is intentional, putting them all to R
                shininess.z = shininessTmp;
            }

            int diffuseTexIdx0 = loadTexture(aiTextureType_DIFFUSE, scene, mat);
            int diffuseTexIdx1 = loadTexture(aiTextureType_HEIGHT, scene, mat);
            int diffuseTexIdx2 = loadTexture(aiTextureType_EMISSIVE, scene, mat);
            int diffuseTexIdx3 = loadTexture(aiTextureType_AMBIENT, scene, mat);
            int specularTexIdx = loadTexture(aiTextureType_SPECULAR, scene, mat);
            int shinyTexIdx = loadTexture(aiTextureType_SHININESS, scene, mat);

            if (ret != AI_SUCCESS || (shininessTmp == 0 && diffuseTexIdx1 == -1)) {
                specular.x = 0; // No shininess
                specular.y = 0;
                specular.z = 0;
            }

            materials.push_back(Material(
                    diffuse,
                    specular,
                    shininess,
                    diffuseTexIdx0,
                    diffuseTexIdx1,
                    diffuseTexIdx2,
                    diffuseTexIdx3,
                    specularTexIdx,
                    shinyTexIdx, true));
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

        int width, height, channels;
        unsigned char *imgData;
        // We still need to handle embedded textures since the character models are in .glb file format
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
            std::cout << "Error in loading the texture image: " << textureAbsolutePath << std::endl;
            return -1;
        }

        // Texture expects a vector, not an array
        std::vector<unsigned char> dataVector(imgData, imgData + width * height * channels);

        // Switch to our type of texture enum
        // To handle alternate textures for each season, we export each texture as alternate texture types
        // from Blender. It's scuffed but it works (probably)
        enum TexType sgeType;
        switch (type) {
            case aiTextureType_DIFFUSE:
                sgeType = DIFFUSE_TEXTURE0;
                break;
            case aiTextureType_HEIGHT:
                sgeType = DIFFUSE_TEXTURE1;
                break;
            case aiTextureType_EMISSIVE:
                sgeType = DIFFUSE_TEXTURE2;
                break;
            case aiTextureType_AMBIENT:
                sgeType = DIFFUSE_TEXTURE3;
                break;
            case aiTextureType_SPECULAR:
                sgeType = SPECULAR_TEXTURE;
                break;
            case aiTextureType_SHININESS:
                sgeType = SHININESS_TEXTURE;
                break;
            default:
                sgeType = UNKNOWN_TEXTYPE;
                break;
        }

        // Add texture to sge data structures
        int ret = textures.size();
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
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, dataVector.data());
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        stbi_image_free(imgData);
        glBindTexture(GL_TEXTURE_2D, 0);
        return ret;
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
            // Load missing bones in animation
            if (!boneMap.count(nodeName)) {
                boneMap[nodeName] = boneMap.size();
                bones.offsetMatrices.push_back(glm::mat4(1)); // To maintain invariant that this vector can always be indexed by bone id, tho this matrix (should) never be used
                numBones++;
            }
            int id = boneMap[nodeName];
            cur.channels[id] = BonePose(*animation.mChannels[i], id);
        }
        return cur;
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
     * @param animationId Model's animation identifier
     * @param time Timestamp in animation ticks to retrieve pose
     * @return Model's pose at given timestamp
     */
    void ModelComposite::animationPose(int animationId, float time, ModelPose& outputModelPose) {
        assert(animationId >= -1  && animationId < (int) animations.size() && animated == true);
        if (animationId == -1) {
            animationId = animationWhenStill;
        }
        Animation anim = animations[animationId];
        glm::mat4 accumulator(1);
        // Recursively construct final transformation matrices for each bone
        recursePose(outputModelPose, anim, time, accumulator, bones.root);
    }

    /**
     * Get pose object for model's default/binding pose
     * @return
     */
    ModelPose ModelComposite::emptyModelPose() {
        ModelPose pose(MAX_BONES, glm::mat4(1));
        return pose;
    }

    /**
     * Convert milliseconds timestamp to animation tick
     * @param milliseconds
     * @param animationId
     * @return
     */
    float ModelComposite::timeToAnimationTick(long long milliseconds, int animationId) {
        assert(animationId >= -1 && animationId < (int) animations.size() && animated == true);
        if (animationId == -1) {
            return animationTickWhenStill;
        }
        Animation anim = animations[animationId];
        float ticks = milliseconds * anim.ticksPerSecond / 1000;
        // apply animation loop
        if (ticks > anim.duration) {
            ticks -= ((int)(ticks / anim.duration)) * anim.duration;
        }
        return ticks;
    }

    /**
     * Render model with a specified pose
     * @param modelPosition Entity position in world coordinates
     * @param modelYaw Model yaw (rotation of model in degrees)
     * @param pose Pose to draw model in
     * @param shadow Whether to draw to shadow map
     * @param outline Whether to draw outline, does nothing if shadow is true
     */
    void ModelComposite::renderPose(const glm::vec3 &modelPosition, const float &modelYaw, ModelPose pose, bool shadow,
                                    bool outline) const {
        if (shadow) {
            shadowProgram.useShader();
            shadowProgram.setAnimated(true);
        } else {
            defaultProgram.useShader();
            defaultProgram.setAnimated(true);
            defaultProgram.updateOutline(outline);
        }

        glBindVertexArray(VAO);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), modelPosition); // This instance's transformation matrix - specifies instance's rotation, translation, etc.
        model = glm::rotate(model, glm::radians(modelYaw), glm::vec3(0.0f, -1.0f, 0.0f));
        // yaw (cursor movement) should rotate our player model AND the camera view, right?
        if (shadow) {
            shadowProgram.updateModelMat(model);
            shadowProgram.updateBoneTransforms(pose);
        } else {
            defaultProgram.updateModelMat(model);
            defaultProgram.updateBoneTransforms(pose);
        }

        // Draw each mesh to the screen
        for (unsigned int i = 0; i < meshes.size(); i++) {
            const Material &mat = materials[meshes[i].MaterialIndex];
            if (shadow == false) {
                mat.setShaderMaterial();
            }
            glDrawElementsBaseVertex(GL_TRIANGLES, meshes[i].NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * meshes[i].BaseIndex), meshes[i].BaseVertex);
//            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glBindVertexArray(0);
    }

    /**
     * Recursively evaluate bone poses at a given time
     *
     * @param out Output parameter - put final pose in this vector
     * @param anim Animation object to evaluate model pose with
     * @param time Timestep to evaluate pose during animation (milliseconds)
     * @param accumulator Accumulator transformation matrix to handle bone hierarchy
     * @param cur Current bone node
     */
    void ModelComposite::recursePose(ModelPose &out, Animation &anim, float time, glm::mat4 accumulator, ModelComposite::BoneNode cur) {
        assert(animated == true);
        if (cur.id == -1) {
            accumulator = accumulator * cur.relativeTransform;
        } else {
            accumulator = accumulator * anim.channels[cur.id].poseAtTime(time);
            out[cur.id] = animationGlobalInverse * accumulator * bones.offsetMatrices[cur.id];
        }
        for (const BoneNode &child : cur.children) {
            recursePose(out, anim, time, accumulator, child);
        }
    }

    /**
     * Whether the model has animations
     * @return
     */
    bool ModelComposite::isAnimated() const {
        return animated;
    }

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
     *
     * Returns pose at final keyframe if time > final keyframe timestamp
     * @param time Time to interpolate position at (milliseconds)
     * @return Bone's transformation matrix at specified time
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

    /**
     * Interpolate bone rotation at a given time
     * Returns final pose if time > last keyframe timestamp
     * @param time Time to evaluate bone rotation pose (milliseconds)
     * @return Rotation matrix at specified time
     */
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

    /**
     * Interpolate bone scale at a given timestamp in an animation
     * Returns final pose if time > last keyframe timestamp
     * @param time Time to evaluate bone scale pose at (milliseconds)
     * @return Scaling matrix at specified time
     */
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
    *
    * @param diffuse
    * @param specular
    * @param shininess
    * @param enableSeasons
    */
   Material::Material(glm::vec3 diffuse, glm::vec3 specular, glm::vec3 shininess, bool enableSeasons) :
           diffuse(diffuse),
           specular(specular),
           shininess(shininess),
           specularMap(-1),
           shinyMap(-1){
       diffuseMap[0] = -1;
       diffuseMap[1] = -1;
       diffuseMap[2] = -1;
       diffuseMap[3] = -1;
       // Enable seasons changing if there are 4 diffuse colors/textures available
       if (enableSeasons == true && (diffuseMap[0] > 0 && diffuseMap[1] > 0 && diffuseMap[2] > 0 && diffuseMap[3] > 0)) {
           seasons = true;
       } else {
           seasons = false;
       }
       multipleTextures = false;
   }

    /**
     *
     * @param diffuse
     * @param specular
     * @param shininess
     * @param diffuseMap0
     * @param diffuseMap1
     * @param diffuseMap2
     * @param diffuseMap3
     * @param specularMap
     * @param shinyMap
     * @param enableSeasons
     */
    Material::Material(glm::vec3 diffuse, glm::vec3 specular, glm::vec3 shininess, int diffuseMap0, int diffuseMap1,
                       int diffuseMap2, int diffuseMap3, int specularMap, int shinyMap, bool enableSeasons) :
           diffuse(diffuse),
            specular(specular),
            shininess(shininess),
            specularMap(specularMap),
            shinyMap(shinyMap){
        diffuseMap[0] = diffuseMap0;
        diffuseMap[1] = diffuseMap1;
        diffuseMap[2] = diffuseMap2;
        diffuseMap[3] = diffuseMap3;

        // Enable seasons changing if there are 4 diffuse colors/textures available
        if (enableSeasons == true && (diffuseMap[0] > -1 && diffuseMap[1] > -1 && diffuseMap[2] > -1 && diffuseMap[3] > -1)) {
            seasons = true;
        } else {
            seasons = false;
        }
        // Enable switching between textures if there are multiple diffuse textures available
        if (diffuseMap[1] > -1) {
            multipleTextures = true;
        } else {
            multipleTextures = false;
        }
    }

    /**
     *
     * @param _diffuse
     * @param specular
     * @param shininess
     * @param _diffuseMap
     * @param specularMap
     * @param shinyMap
     */
    Material::Material(glm::vec3 _diffuse, glm::vec3 specular, glm::vec3 shininess, int _diffuseMap, int specularMap,
                       int shinyMap, bool enableSeasons)
            : specular(specular), shininess(shininess), specularMap(specularMap), shinyMap(shinyMap) {
        diffuse = _diffuse;
        diffuseMap[0] = _diffuseMap;
        multipleTextures = false;
        seasons = false;
        multipleTextures = false;
    }

   /**
    * Tell active shader about material properties to render
    */
    void Material::setShaderMaterial() const {
       if (diffuseMap[0] != -1) {
           // Tell shader there is a diffuse map
           glUniform1i(defaultProgram.hasDiffuseMap, 1);
           if (seasons || multipleTextures) {
               glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE0);
               glBindTexture(GL_TEXTURE_2D, texID[diffuseMap[0]]);
               glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE1);
               glBindTexture(GL_TEXTURE_2D, texID[diffuseMap[1]]);
               glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE2);
               glBindTexture(GL_TEXTURE_2D, texID[diffuseMap[2]]);
               glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE3);
               glBindTexture(GL_TEXTURE_2D, texID[diffuseMap[3]]);
           } else {
               glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE0);
               glBindTexture(GL_TEXTURE_2D, texID[diffuseMap[0]]);
           }
       } else {
           // Tell shader there is no diffuse map
           glUniform1i(defaultProgram.hasDiffuseMap, 0);
           glUniform3fv(defaultProgram.diffuseColor, 1, &diffuse[0]);
       }

       // Tell shader if the current material supports multiple textures or seasons
       glUniform1i(defaultProgram.seasons, seasons);
       glUniform1i(defaultProgram.alternating, multipleTextures);

       if (specularMap != -1) {
           glUniform1i(defaultProgram.hasSpecularMap, 1);
           glActiveTexture(GL_TEXTURE0 + SPECULAR_TEXTURE);
           glBindTexture(GL_TEXTURE_2D, texID[specularMap]);
       } else {
           glUniform1i(defaultProgram.hasSpecularMap, 0);
           glUniform3fv(defaultProgram.specularColor, 1, &specular[0]);
       }

       if (shinyMap != -1) {
           glUniform1i(defaultProgram.hasShinyMap, 1);
           glActiveTexture(GL_TEXTURE0 + SHININESS_TEXTURE);
           glBindTexture(GL_TEXTURE_2D, texID[shinyMap]);
       } else {
           glUniform1i(defaultProgram.hasShinyMap, 0);
           glUniform3fv(defaultProgram.shinyColor, 1, &shininess[0]);
       }
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

    /**
     * Updates camera lookat matrix - the lookat matrix transforms vertices from world coordinates to camera coordinates
     * WARNING: THIS WILL CHANGE THE ACTIVE SHADER PROGRAM
     * @param playerPosition Player position
     * @param yaw Camera yaw
     * @param pitch Camera pitch
     * @param distanceBehind Distance behind player
     */
    void updateCameraToFollowPlayer(glm::vec3 playerPosition, float yaw, float pitch, float distanceBehind) {
        // the camera and the player should face the same direction (?)
        cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraDirection.y = sin(glm::radians(pitch));
        cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        // the camera is somewhere behind the player
        cameraPosition = playerPosition - (cameraDirection * distanceBehind);
        // move camera above player for better view (it automatically becomes 'fps' when distanceBehind<0)
        cameraPosition.y += CAMERA_DISTANCE_ABOVE_PLAYER;

        // Send camera position to shaders
        cameraUp = glm::cross(glm::cross(cameraDirection, glm::vec3(0, 1, 0)), cameraDirection);
        viewMat = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);

        particleProgram.useShader();
        particleProgram.updateViewMat(viewMat);

        defaultProgram.useShader();
        defaultProgram.updateCamPos(cameraPosition);
        defaultProgram.updateViewMat(viewMat);
        
        lineShaderProgram.updateViewMat(viewMat);
    }

    /**
     * Gracefully deallocate textures in OpenGL context
     */
    void deleteTextures() {
        if (texID.size() > 0) {
            glDeleteTextures(texID.size(), &texID[0]);
        }
    }

    /**
     * Create a new emitter to render with
     * NOTE: we only really need one emitter for the entire
     * game at the moment
     */
    ParticleEmitter::ParticleEmitter() {
        initBuffers();
    }

    /**
     * Destructor for particle emitter, deletes VAO's, VBO's etc. from OpenGL context
     */
    ParticleEmitter::~ParticleEmitter() {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &CBO);
        glDeleteBuffers(1, &TBO);
        glDeleteVertexArrays(1, &VAO);
    }

    /**
     * Draw count number of particles using the state object
     * @param state Particle state to render
     * @param count Number of particles to render
     */
    void ParticleEmitter::render(ParticleEmitterState &state, size_t count) {
        assert(count <= MAX_PARTICLE_INSTANCE && count <= state.colors.size() && count <= state.transforms.size());
        if (count == 0) return;
        particleProgram.updateParticleSize(state.baseParticleSize);
        // Bind the Vertex Array Object
        glBindVertexArray(VAO);

        // Update the colors buffer
        glBindBuffer(GL_ARRAY_BUFFER, CBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4) * count, &state.colors[0]);

        // Update the transforms buffer
        glBindBuffer(GL_ARRAY_BUFFER, TBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * count, &state.transforms[0][0]);

        // Draw the particles as instanced points
        glDrawArraysInstanced(GL_POINTS, 0, 1, count);
    }

    /**
     * Initialize OpenGL buffers for particle renderer
     */
    void ParticleEmitter::initBuffers() {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glGenBuffers(1, &CBO);
        glBindBuffer(GL_ARRAY_BUFFER, CBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * MAX_PARTICLE_INSTANCE, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        glVertexAttribDivisor(1, 1);

        glGenBuffers(1, &TBO);
        glBindBuffer(GL_ARRAY_BUFFER, TBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * MAX_PARTICLE_INSTANCE, nullptr, GL_DYNAMIC_DRAW);
        // Each mat4 attribute is split into 4 vec4 attributes
        for (int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(2 + i);
            glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(2 + i, 1); // This tells OpenGL this attribute is per-instance
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    /**
     * Default constructor
     */
    ModelComposite::BonePose::BonePose() {
        boneId = -1;
    }

    /**
     * Particle emitter state constructor
     */
    ParticleEmitterState::ParticleEmitterState() {
        baseParticleSize = 0;
    }

    /**
     * Alternate constructor for particle emitter state
     * @param particleSize
     */
    ParticleEmitterState::ParticleEmitterState(float particleSize) {
        baseParticleSize = particleSize;
    }

    glm::vec3 cameraPosition;
    glm::vec3 cameraDirection;
    glm::vec3 cameraUp;
    glm::mat4 perspectiveMat;
    glm::mat4 viewMat;

    // For some reason these only work if they're unique pointers, i don't know why
    // we roll with it
    std::vector<std::unique_ptr<ParticleEmitter>> emitters;
    std::vector<std::unique_ptr<ModelComposite>> models;

    std::vector<Texture> textures;
    std::vector<GLuint> texID; // OpenGL texture identifiers
}
