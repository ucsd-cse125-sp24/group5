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
            std::unique_ptr<Entity> createEntity();
            void addSystem(System* system);
            void destroyEntity(Entity e);

            template<typename ComponentType>
            void addComponent(std::unique_ptr<Entity>& e, std::unique_ptr<ComponentType>& c);
  
            template<typename ComponentType>
            void removeComponent(std::unique_ptr<Entity>& e, std::unique_ptr<ComponentType>& c);

            void movePlayer(unsigned int player, float x, float y, float z);

            void printDebug();

            Entity createEntityTest();

        private:
            // std::unique_ptr<std::vector<System*>> systems;
            // std::unique_ptr<std::set<std::unique_ptr<Entity>>> entities;
            // int currMaxEntityId;
            // std::array<std::unique_ptr<Entity>, NUM_MOVEMENT_ENTITIES> players;
            // std::unique_ptr<ComponentManager<PositionComponent>> positionCM;

            std::vector<System> systems;
            std::set<Entity> entities;
            int currMaxEntityId;
            Entity players[NUM_MOVEMENT_ENTITIES];
            ComponentManager<PositionComponent> positionCM;
    };

}