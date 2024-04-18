#include "bge/World.h"

namespace bge {

    void World::init() {

    }

    Entity* World::createEntity() {

        Entity* newEntity = new Entity();
        newEntity->id = currMaxEntityId;
        currMaxEntityId++;

        entities.insert(newEntity);

        return newEntity;

    }

    void World::addSystem(System* system) {

        systems.push_back(system);

    }

    void World::destroyEntity(Entity e) {


    }

    template<typename ComponentType>
    void World::addComponent(Entity e, ComponentType c) {

    }

    template<typename ComponentType>
    void World::removeComponent(Entity e, ComponentType c) {
        
    }

    void World::printEntities() {
        std::cout << "Printing Entities" << std::endl;
        for (Entity* e : entities) {
            std::cout << e->id << std::endl; 
        }
    }

} 