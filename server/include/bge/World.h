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
            Entity* createEntity();
            void addSystem(System* system);
            void destroyEntity(Entity e);

            template<typename ComponentType>
            void addComponent(Entity* e, ComponentType* c);
  
            template<typename ComponentType>
            void removeComponent(Entity* e, ComponentType* c);

            void movePlayer(unsigned int player, float x, float y, float z);

            void printDebug();

        private:
            std::vector<System*> systems;
            std::set<Entity*> entities;
            int currMaxEntityId;
            Entity* players[NUM_MOVEMENT_ENTITIES];

            ComponentManager<PositionComponent>* positionCM;
    };

}