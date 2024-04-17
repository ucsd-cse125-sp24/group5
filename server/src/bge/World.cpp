#include "bge/World.h"

namespace bge {

    void World::init() {

    }

    int World::test() {
        return 1;
    }

    Entity World::createEntity() {

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