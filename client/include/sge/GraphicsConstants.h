//
// Created by benjx on 5/4/2024.
//

#pragma once

/**
 * Texture types
 */
enum TexType {
    DIFFUSE_TEXTURE = 0,
    SPECULAR_TEXTURE = 1,
    BUMP_MAP = 2,
    DISPLACEMENT_MAP = 3,
    SHININESS_TEXTURE = 4,
    UNKNOWN_TEXTYPE = 5,
    NUM_TEXTURES = 6
};

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


// Maximum number of bones that may influence a single vertex
#define MAX_BONE_INFLUENCE 4

// Maximum number of bones per model
#define MAX_BONES 100

// Assimp model importer flags
#define ASSIMP_IMPORT_FLAGS aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_EmbedTextures | aiProcess_GenNormals | aiProcess_FixInfacingNormals | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_ValidateDataStructure | aiProcess_FindInstances | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes
