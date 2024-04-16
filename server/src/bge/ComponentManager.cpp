#include "ComponentManager.h"

namespace bge {

    template <typename ComponentType>
    bge::ComponentManager<ComponentType>::ComponentManager() {
        componentData = ComponentData();
        entityMap = new map<Entity, ComponentIndex>();
        componentMap = new map<ComponentIndex, Entity>();
    }

    template <typename ComponentType>
    ComponentIndex ComponentManager<ComponentType>::add(Entity e, ComponentType& c) {
        ComponentIndex newIndex = { componentData.size };
        componentData.data[newIndex] = c;
        componentData.size++;

        entityMap[e] = newIndex;
        componentMap[newIndex] = e;

        return newIndex;
    }

    template <typename ComponentType>
    ComponentType* ComponentManager<ComponentType>::lookup(Entity e) {
        ComponentIndex index = entityMap[e];
        return &componentData.data[index.index];
    }

    template <typename ComponentType>
    void ComponentManager<ComponentType>::destroy(Entity e) {
        ComponentIndex index = entityMap[e];
        ComponentIndex lastComponent = componentData.size - 1;
        componentData[index] = componentData.data[lastComponent];
        componentData.size--;
  
        // Update the map for the moved entity as well
        Entity movedEntity = componentMap[lastComponent];
        componentMap[index] = movedEntity;
        entityMap[movedEntity] = index;
    }

}