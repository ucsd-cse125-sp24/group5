#pragma once

#include "Entity.h"
#include "System.h"
#include "ComponentManager.h"
#include "GameConstants.h"
#include "NetworkData.h"
#include <set>
#include <iostream>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

#define ASSIMP_IMPORT_FLAGS aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_EmbedTextures | aiProcess_GenNormals | aiProcess_FixInfacingNormals | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_ValidateDataStructure | aiProcess_FindInstances | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes

#define MAP_BUCKET_WIDTH 20

struct rayIntersection {
    float t;
    glm::vec3 normal;
    uint32_t tri;
};

namespace bge {

    class System;

    class World {
        public:
            void init();
            Entity createEntity();

            // One function for each component type, since the alternatives involve crazy c++ that probably doesn't even work
            void addComponent(Entity e, PositionComponent c);
            void addComponent(Entity e, VelocityComponent c);
            void addComponent(Entity e, MovementRequestComponent c);
            void addComponent(Entity e, JumpInfoComponent c);

            // No idea why we can do the simpler definition for deleteComponent but we can't for addComponent
            template<typename ComponentType>
            void deleteComponent(Entity e, ComponentType c);

            void updateAllSystems();

            // This can't be contained within a system since we want to do this as we receive client packets rather than once per tick
            void updatePlayerInput(unsigned int player, float pitch, float yaw, bool forwardRequested, bool backwardRequested, bool leftRequested, bool rightRequested, bool jumpRequested);

            void fillInGameData(ServerToClientPacket& packet);

            void printDebug();

            rayIntersection intersect(glm::vec3 p0, glm::vec3 p1, float maxT);

        private:
            void initMesh();
            std::vector<unsigned int> determineBucket(float x, float z);
            std::vector<glm::vec3> mapVertices;
            // Outer vector holds each bucket (MAP_BUCKET_WDITH * MAP_BUCKET_WIDTH of them)
            // Within each bucket, the uint32_ts refer to indices into the vertex array
            // (3 in a row give you a triangle)
            std::vector<uint32_t> mapTriangles;
            std::vector<std::unordered_set<unsigned int>>buckets = std::vector<std::unordered_set<unsigned int>>(MAP_BUCKET_WIDTH * MAP_BUCKET_WIDTH, std::unordered_set<unsigned int>());
            float minMapXValue;
            float maxMapXValue;
            float minMapZValue;
            float maxMapZValue;

            std::vector<std::shared_ptr<System>> systems;
            std::set<Entity> entities;
            int currMaxEntityId;

            std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
            std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
            std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM;
            std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM;

            Entity players[NUM_MOVEMENT_ENTITIES];
    };

}