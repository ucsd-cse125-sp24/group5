#pragma once

#include "Entity.h"
#include "System.h"
#include "ComponentManager.h"
#include <set>

namespace bge {

    class World {
        public:
            void init();
            Entity createEntity();
            void addSystem(System * system);
            void destroyEntity(Entity e);

            template<typename ComponentType>
            void addComponent(Entity& e, ComponentType && c);
  
            template<typename ComponentType>
            void removeComponent(Entity& e);

        private:
            std::vector<System*> systems;
            std::set<Entity*> entities;
            int numEntities;
    };

}