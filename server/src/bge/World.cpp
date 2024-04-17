#include "bge/World.h"

namespace bge {

    void World::init() {

    }

    Entity World::createEntity() {

        Entity newEntity;
        newEntity.id = numEntities;
        numEntities++;
        return newEntity;

    }

    void World::addSystem(System * system) {

    }

    void World::destroyEntity(Entity e) {

    }

    template<typename ComponentType>
    void World::addComponent(Entity& e, ComponentType && c) {

    }

    template<typename ComponentType>
    void World::removeComponent(Entity& e) {

    }

} 