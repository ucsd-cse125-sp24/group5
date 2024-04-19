#pragma once
#include <map>
#include <array>
#include "Entity.h"

namespace bge {

    template <typename ComponentType>
    class ComponentManager {
        public:
            ComponentManager() {
                componentDataStorage = std::make_unique<std::vector<std::unique_ptr<ComponentType>>>(64);                
                entityMap = std::make_unique<std::map<int, int>>();
                componentMap = std::make_unique<std::map<int, int>>();
                size = 0;
            }
            int add(std::unique_ptr<Entity>& e, std::unique_ptr<ComponentType>& c) {
                int newIndex = size;
                (*componentDataStorage)[newIndex] = std::move(c);
                size++;

                (*entityMap)[e->id] = newIndex;
                (*componentMap)[newIndex] = e->id;

                return newIndex;
            }
            std::unique_ptr<ComponentType>& lookup(std::unique_ptr<Entity>& e) {
                int index = (*entityMap)[e->id];
                return (*componentDataStorage)[index];
            }
            void destroy(Entity* e) {
                int removedIndex = (*entityMap)[e->id];
                int lastComponentIndex = size - 1;
                (*componentDataStorage)[removedIndex] = (*componentDataStorage)[lastComponentIndex];
                size--;
  
                // Update the map for the moved entity as well
                int movedEntity = (*componentMap)[lastComponentIndex];
                (*componentMap)[removedIndex] = movedEntity;
                (*entityMap)[movedEntity] = removedIndex;
            }
        private:
            std::unique_ptr<std::vector<std::unique_ptr<ComponentType>>> componentDataStorage;
            // Entity id to index in storage
            std::unique_ptr<std::map<int, int>> entityMap;
            // Store index to Entity id
            std::unique_ptr<std::map<int, int>> componentMap;
            int size;
    };

} 