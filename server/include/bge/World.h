#pragma once

#include "Entity.h"
#include "System.h"
#include "ComponentManager.h"
#include "GameConstants.h"
#include <set>
#include <iostream>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>

namespace bge {

    class World {
        public:
            void init();
            Entity createEntity();

            // One function for each component type, since the alternatives involve crazy c++ that probably doesn't even work
            void addComponent(Entity e, PositionComponent c);
            void addComponent(Entity e, VelocityComponent c);
            void addComponent(Entity e, MovementRequestComponent c);

            /* void deleteComponent(Entity e, PositionComponent c);
            void deleteComponent(Entity e, VelocityComponent c);
            void deleteComponent(Entity e, MovementRequestComponent c);*/
            template<typename ComponentType>
            void deleteComponent(Entity e, ComponentType c);

            void updateAllSystems();

            void movePlayer(unsigned int player, float x, float y, float z);

            void printDebug();

        private:
            std::vector<System> systems;
            std::set<Entity> entities;
            int currMaxEntityId;

            ComponentManager<PositionComponent> positionCM;
            ComponentManager<VelocityComponent> velocityCM;
            ComponentManager<MovementRequestComponent> movementRequestCM;

            Entity players[NUM_MOVEMENT_ENTITIES];
    };

}