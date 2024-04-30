#pragma once

#include "Entity.h"
#include "System.h"
#include "ComponentManager.h"
#include "GameConstants.h"
#include "NetworkData.h"
#include "EventHandler.h"

#include <set>
#include <iostream>

namespace bge {

    class World {
        public:
            void init();

            Entity createEntity(EntityType type);
            void deleteEntity(Entity entity);
            void createProjectile();

            // One function for each component type, since the alternatives involve crazy c++ that probably doesn't even work
            void addComponent(Entity e, PositionComponent c);
            void addComponent(Entity e, VelocityComponent c);
            void addComponent(Entity e, JumpInfoComponent c);
            void addComponent(Entity e, MovementRequestComponent c);
            void addComponent(Entity e, HealthComponent c);
            void addComponent(Entity e, DimensionComponent c);
            void addComponent(Entity e, EggHolderComponent c);
            void addComponent(Entity e, PlayerDataComponent c);
            // No idea why we can do the simpler definition for deleteComponent but we can't for addComponent
            template<typename ComponentType>
            void deleteComponent(Entity e, ComponentType c);

            void updateAllSystems();

            // This can't be contained within a system since we want to do this as we receive client packets rather than once per tick
            void updatePlayerInput(unsigned int player, float pitch, float yaw, bool forwardRequested, bool backwardRequested, bool leftRequested, bool rightRequested, bool jumpRequested);

            void fillInGameData(ServerToClientPacket& packet);

            void printDebug();

        private:
            std::vector<std::shared_ptr<System>> systems;
            std::set<Entity> entities;
            int currMaxEntityId;

            // we need position and lastPosition to calculate ray pathing
            std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
            std::shared_ptr<ComponentManager<PositionComponent>> lastPositionCM;

            std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
            std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM;
            std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM;
            std::shared_ptr<ComponentManager<HealthComponent>> healthCM;
            std::shared_ptr<ComponentManager<DimensionComponent>> dimensionCM;
            std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM;
            std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataCM;

            std::shared_ptr<ProjectileVsPlayerHandler> projectileVsPlayerHandler;
            std::shared_ptr<EggVsPlayerHandler> eggVsPlayerHandler;

            Entity players[NUM_PLAYER_ENTITIES];
            Entity egg;
            // trees, rocks, and house in the map --- AABBs?
    };

}