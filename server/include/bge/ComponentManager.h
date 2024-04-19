#pragma once
#include <map>
#include <array>
#include "Entity.h"

namespace bge {

    template <typename ComponentType>
    class ComponentManager {
        public:
            ComponentManager() {
                componentDataStorage = std::vector<ComponentType>();  
                componentDataStorage.reserve(256);             
                entityMap = std::map<int, int>();
                componentMap = std::map<int, int>();
                size = 0;
            }
            int add(Entity e, ComponentType c) {
                int newIndex = size;
                componentDataStorage[newIndex] = c;
                size++;

                entityMap[e.id] = newIndex;
                componentMap[newIndex] = e.id;

                return newIndex;
            }
            ComponentType& lookup(Entity e) {
                int index = entityMap[e.id];
                return componentDataStorage[index];
            }
        private:
            std::vector<ComponentType> componentDataStorage;
            // Entity id to index in storage
            std::map<int, int> entityMap;
            // Store index to Entity id
            std::map<int, int> componentMap;
            int size;
    };

} 