#include "bge/World.h"

namespace bge {

    void World::init() {

        // positionCM = std::make_unique<bge::ComponentManager<PositionComponent>>();

        // for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
        //     std::unique_ptr<Entity> newPlayer = createEntity();
        //     players[i] = std::move(newPlayer);
        //     std::unique_ptr<PositionComponent> pos = std::make_unique<PositionComponent>(i*10.0f, 3.0f, -(i%2)*8.0f);
        //     std::cout << i*10.0f << 3.0f << -(i%2)*8.0f << std::endl;
        //     addComponent(newPlayer, pos);
        // }

        positionCM = ComponentManager<PositionComponent>();
        for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
            Entity newPlayer = createEntityTest();
            players[i] = newPlayer;
            PositionComponent pos = PositionComponent(i*10.0f, 3.0f, -(i%2)*8.0f);
        }
    }

    std::unique_ptr<Entity> World::createEntity() {

        // std::unique_ptr<Entity> newEntity = std::make_unique<Entity>();
        // newEntity->id = currMaxEntityId;
        // currMaxEntityId++;

        // // entities->push_back(std::move(newEntity));

        // return newEntity;

    }

    Entity World::createEntityTest() {
        Entity newEntity = Entity();
        newEntity.id = currMaxEntityId;
        currMaxEntityId++;
        entities.insert(newEntity);
        return newEntity;
    }

    void World::addSystem(System* system) {


    }

    void World::destroyEntity(Entity e) {


    }

    template<typename ComponentType>
    void World::addComponent(std::unique_ptr<Entity>& e, std::unique_ptr<ComponentType>& c) {
       // positionCM->add(e, c);
    }

    template<typename ComponentType>
    void World::removeComponent(std::unique_ptr<Entity>& e, std::unique_ptr<ComponentType>& c) {
        
    }

    void World::printDebug() {
        // std::cout << "Printing Entities" << std::endl;
        // for (Entity* e : entities) {
        //     std::cout << e->id << std::endl; 
        // }
        // for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
        //     std::unique_ptr<PositionComponent>& pos = positionCM->lookup(players[i]);
        //     std::printf("ECS position x(%f) y(%f) z(%f)\n", pos->position.x, pos->position.y, pos->position.z);
        // }
    }

    void World::movePlayer(unsigned int player, float x, float y, float z) {

        // Check x, y, z for valid movement
        // std::unique_ptr<PositionComponent>& pos = positionCM->lookup(players[player]);
        // pos->position = glm::vec3(x,y,z);

    }

} 