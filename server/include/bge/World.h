#pragma once

#include "Entity.h"
#include "System.h"
#include "ComponentManager.h"
#include "GameConstants.h"
#include "NetworkData.h"

#include <time.h> 
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

#define MAP_BUCKET_WIDTH 40

struct rayIntersection {
    float t;
    glm::vec3 normal;
    bge::Entity ent;    // entity being hit
};

namespace bge {

    class System;

    class World {
        public:
            void init();

            Entity createEntity(EntityType type);
            void deleteEntity(Entity entity);
            Entity getFreshProjectile(BallProjType projType);

            // One function for each component type, since the alternatives involve crazy c++ that probably doesn't even work
            void addComponent(Entity e, PositionComponent c);
            void addComponent(Entity e, MeshCollisionComponent c);
            void addComponent(Entity e, VelocityComponent c);
            void addComponent(Entity e, JumpInfoComponent c);
            void addComponent(Entity e, MovementRequestComponent c);
            void addComponent(Entity e, HealthComponent c);
            void addComponent(Entity e, BoxDimensionComponent c);
            void addComponent(Entity e, EggHolderComponent c);
            void addComponent(Entity e, PlayerDataComponent c);
            void addComponent(Entity e, CameraComponent c);
            void addComponent(Entity e, StatusEffectsComponent c);
            void addComponent(Entity e, SeasonAbilityStatusComponent c);
            void addComponent(Entity e, BallProjDataComponent c);
            // Component managers
            std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
            std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
            std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM;
            std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM;
            std::shared_ptr<ComponentManager<HealthComponent>> healthCM;
            std::shared_ptr<ComponentManager<BoxDimensionComponent>> boxDimensionCM;
            std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM;
            std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataCM;
            std::shared_ptr<ComponentManager<MeshCollisionComponent>> meshCollisionCM;
            std::shared_ptr<ComponentManager<CameraComponent>> cameraCM;
            std::shared_ptr<ComponentManager<StatusEffectsComponent>> statusEffectsCM;
            std::shared_ptr<ComponentManager<SeasonAbilityStatusComponent>> seasonAbilityStatusCM;
            std::shared_ptr<ComponentManager<BallProjDataComponent>> ballProjDataCM;

            // No idea why we can do the simpler definition for deleteComponent but we can't for addComponent
            template<typename ComponentType>
            void deleteComponent(Entity e, ComponentType c);

            void updateAllSystems();

            // This can't be contained within a system since we want to do this as we receive client packets rather than once per tick
            void updatePlayerInput(unsigned int player, float pitch, float yaw, bool forwardRequested, bool backwardRequested, bool leftRequested, bool rightRequested, bool jumpRequested, bool throwEggRequested, bool shootRequested, bool abilityRequested);

            void fillInGameData(ServerToClientPacket& packet);
            void fillInBulletData(BulletPacket& packet);

            void printDebug();
            Entity getEgg();

            bool withinMapBounds(glm::vec3 pos);

            rayIntersection intersect(glm::vec3 p0, glm::vec3 p1, float maxT);
            rayIntersection intersectRayBox(glm::vec3 origin, glm::vec3 direction, float maxT);
            // nice to have: intersectRaySphere() to let dome shield block bullets

            std::vector<BulletTrail> bulletTrails;

            glm::vec3 voidLocation;

            Entity players[NUM_PLAYER_ENTITIES];

            int currentSeason;

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
            float minMapYValue;
            float maxMapYValue;

            std::vector<std::shared_ptr<System>> systems;
            std::set<Entity> entities;
            int currMaxEntityId;

            Entity egg;
            Entity ballProjectiles[NUM_PROJ_TYPES][NUM_EACH_PROJECTILE];
    };

}