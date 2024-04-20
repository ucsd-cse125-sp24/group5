#pragma once

#include "Entity.h"
#include "System.h"
#include "ComponentManager.h"
#include "GameConstants.h"
#include <set>
#include <iostream>

namespace bge {

    class World {
        public:
            void init();
            Entity createEntity();

            template<typename ComponentType>
            void addComponent(Entity e, ComponentType c);

            template<typename ComponentType>
            void deleteComponent(Entity e, ComponentType c);

            void movePlayer(unsigned int player, float x, float y, float z);

            void printDebug();

        private:
            std::vector<System> systems;
            std::set<Entity> entities;
            int currMaxEntityId;

            ComponentManager<PositionComponent> positionCM;

            Entity players[NUM_MOVEMENT_ENTITIES];
    };

}