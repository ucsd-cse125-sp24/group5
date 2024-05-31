//
// Created by benjx on 5/4/2024.
//

#pragma once

/**
 * Texture types
 */
enum TexType {
    DIFFUSE_TEXTURE0 = 0,
    DIFFUSE_TEXTURE1 = 1,
    DIFFUSE_TEXTURE2 = 2,
    DIFFUSE_TEXTURE3 = 3,
    SPECULAR_TEXTURE = 4,
    SHININESS_TEXTURE = 5,
    SHADOWMAP_TEXTURE = 6,
    UNKNOWN_TEXTYPE = 7,
    NUM_TEXTURES = 8
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
    BEAR = 1, // currently we can only make players be foxes, not bears, since we don't have all animations for the bears yet
    FOX = 2,
    EGG = 3,
    SPRING_BALL = 4,
    SUMMER_BALL = 5,
    AUTUMN_BALL = 6,
    WINTER_BALL = 7,
    NUM_MODELS
};

enum ParticleIndex {
    FIRE = 0,
    SNOW = 1,
    NUM_PARTICLES
};

// SGE_SPRING or whatever because GameConstants.h defines SPRING, SUMMER, AUTUMN, etc. and I don't want any possible weird stuff
// happening if those enums get changed
enum Season {
    SGE_SPRING = 0,
    SGE_SUMMER = 1,
    SGE_AUTUMN = 2,
    SGE_WINTER = 3,
    NUM_SEASONS
};


// Maximum number of bones that may influence a single vertex
#define MAX_BONE_INFLUENCE 4

// Maximum number of bones per model
#define MAX_BONES 100

#define MAX_PARTICLE_INSTANCE 1000 // Maximum number of particle instances per particle type

// Assimp model importer flags
#define ASSIMP_IMPORT_FLAGS aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_ValidateDataStructure | aiProcess_FindInstances | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes
