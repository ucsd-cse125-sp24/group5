#pragma once

#include "Entity.h"
#include "System.h"
#include "ComponentManager.h"

namespace bge {

    class World {
        public:
            void init();
            int test();
            Entity createEntity();
            void addSystem(System * system);
            void destroyEntity(Entity e);

            template<typename ComponentType>
            void addComponent(Entity& e, ComponentType && c);
  
            template<typename ComponentType>
            void removeComponent(Entity& e);

        private:
            std::vector<System*> systems;
    };

}