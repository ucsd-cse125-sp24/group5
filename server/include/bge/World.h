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

#define ASSIMP_IMPORT_FLAGS aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_EmbedTextures | aiProcess_GenNormals | aiProcess_FixInfacingNormals | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_ValidateDataStructure | aiProcess_FindInstances | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes

struct rayIntersection {
    float t;
    glm::vec3 normal;
};

namespace bge {

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

        private:
            void initMesh();
            
            rayIntersection intersect(glm::vec3 p0, glm::vec3 p1);
            std::vector<glm::vec3> mapVertices;
            std::vector<uint16_t> mapTriangles;

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