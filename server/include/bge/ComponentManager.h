#pragma once
#include <map>
#include <array>
#include "Entity.h"

namespace bge {

    template <typename ComponentType>
    class ComponentManager {
        public:
            ComponentManager() {
                componentDataStorage = new std::vector<ComponentType*>(64);
                entityMap = new std::map<int, int>();
                componentMap = new std::map<int, int>();
                size = 0;
            }
            ~ComponentManager() {
                
            }
            int add(Entity* e, ComponentType* c) {
                int newIndex = size;
                (*componentDataStorage)[newIndex] = c;
                size++;

                (*entityMap)[e->id] = newIndex;
                (*componentMap)[newIndex] = e->id;

                return newIndex;
            }
            ComponentType* lookup(Entity* e) {
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
            std::vector<ComponentType*>* componentDataStorage;
            // Entity id to index in storage
            std::map<int, int>* entityMap;
            // Store index to Entity id
            std::map<int, int>* componentMap;
            int size;
    };

} 