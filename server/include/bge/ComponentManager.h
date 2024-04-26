#pragma once
#include <unordered_map>
#include <array>
#include <vector>
#include <iostream>
#include "Entity.h"

namespace bge {

    template <typename ComponentType>
    class ComponentManager {
        public:
            ComponentManager() {
                componentDataStorage.reserve(256);
            }
            int add(Entity e, ComponentType c) {
                int newIndex = componentDataStorage.size();
                componentDataStorage.push_back(c);

                entityMap[e.id] = newIndex;
                componentMap[newIndex] = e.id;

                return newIndex;
            }
            void remove(Entity e) {
                int currSize = componentDataStorage.size()-1;
                int delIndex = entityMap[e.id];
                componentDataStorage[delIndex] = componentDataStorage[currSize];
                componentDataStorage.pop_back();

                int movedId = componentMap[currSize];
                entityMap[movedId] = delIndex;
                componentMap[delIndex] = movedId;
            }
            ComponentType& lookup(Entity e) {
                int index = entityMap[e.id];
                return componentDataStorage[index];
            }

            // check if the entity belong exist in this ComponentManager
            bool checkExist(Entity e) {
                auto found = entityMap.find(e.id);
                return found != entityMap.end();
            }

            std::vector<ComponentType>& getAllComponents() {
                return componentDataStorage;
            }
        private:
            std::vector<ComponentType> componentDataStorage;
            // Entity id to index in storage
            std::unordered_map<int, int> entityMap;
            // Store index to Entity id
            std::unordered_map<int, int> componentMap;
    };

} 