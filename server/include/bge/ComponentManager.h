#pragma once
#include <map>
#include <array>
#include "Entity.h"

namespace bge {

    template <typename ComponentType>
    struct ComponentDataStorage {
        unsigned int size = 0;
        std::array<ComponentType, 1024> data;
    };

    template <typename ComponentType>
    class ComponentManager {
        public:
            ComponentManager();
            int add(Entity e, ComponentType& c);
            ComponentType* lookup(Entity e);
            void destroy(Entity e);
        private:
            ComponentDataStorage<ComponentType> componentDataStorage;
            std::map<Entity, int> entityMap;
            std::map<int, Entity> componentMap;
    };

} 