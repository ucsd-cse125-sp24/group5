#include "bge/World.h"

namespace bge {

    void World::init() {

        positionCM = new bge::ComponentManager<PositionComponent>();

        for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
            Entity* newPlayer = this->createEntity();
            players[i] = newPlayer;
            PositionComponent* pos = new PositionComponent(i*10.0f, 3.0f, -(i%2)*8.0f);
            std::cout << i*10.0f << 3.0f << -(i%2)*8.0f << std::endl;
            this->addComponent(newPlayer, pos);
        }
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
    void World::addComponent(Entity* e, ComponentType* c) {
        positionCM->add(e, c);
    }

    template<typename ComponentType>
    void World::removeComponent(Entity* e, ComponentType* c) {
        
    }

    void World::printDebug() {
        // std::cout << "Printing Entities" << std::endl;
        // for (Entity* e : entities) {
        //     std::cout << e->id << std::endl; 
        // }
        for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
            PositionComponent* pos = positionCM->lookup(players[i]);
            std::printf("ECS position x(%f) y(%f) z(%f)\n", pos->position.x, pos->position.y, pos->position.z);
        }
    }

    void World::movePlayer(unsigned int player, float x, float y, float z) {

        // Check x, y, z for valid movement
        PositionComponent* pos = positionCM->lookup(players[player]);
        pos->position = glm::vec3(x,y,z);

    }

} 