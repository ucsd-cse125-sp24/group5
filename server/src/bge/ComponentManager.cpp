#include "bge/ComponentManager.h"

namespace bge {

    template <typename ComponentType>
    bge::ComponentManager<ComponentType>::ComponentManager() {
        componentDataStorage = ComponentDataStorage<ComponentType>();
        entityMap = new std::map<Entity, int>();
        componentMap = new std::map<int, Entity>();
    }

    template <typename ComponentType>
    int ComponentManager<ComponentType>::add(Entity e, ComponentType& c) {
        int newIndex = { componentDataStorage.size };
        componentDataStorage.data[newIndex] = c;
        componentDataStorage.size++;

        entityMap[e] = newIndex;
        componentMap[newIndex] = e;

        return newIndex;
    }

    template <typename ComponentType>
    ComponentType* ComponentManager<ComponentType>::lookup(Entity e) {
        int index = entityMap[e];
        return &componentDataStorage.data[index];
    }

    template <typename ComponentType>
    void ComponentManager<ComponentType>::destroy(Entity e) {
        int removedIndex = entityMap[e];
        int lastComponentIndex = componentDataStorage.size - 1;
        componentDataStorage.data[removedIndex] = componentDataStorage.data[lastComponentIndex];
        componentDataStorage.size--;
  
        // Update the map for the moved entity as well
        Entity movedEntity = componentMap[lastComponentIndex];
        componentMap[removedIndex] = movedEntity;
        entityMap[movedEntity] = removedIndex;
    }

}