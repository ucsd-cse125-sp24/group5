#include "bge/World.h"

namespace bge {

    void World::init() {
        positionCM = ComponentManager<PositionComponent>();
        for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
            Entity newPlayer = createEntity();
            players[i] = newPlayer;
            PositionComponent pos = PositionComponent(i*10.0f, 3.0f, -(i%2)*8.0f);
            addComponent(newPlayer, pos);
        }
    }

    Entity World::createEntity() {
        Entity newEntity = Entity();
        newEntity.id = currMaxEntityId;
        currMaxEntityId++;
        entities.insert(newEntity);
        return newEntity;
    }

    template<typename ComponentType>
    void World::addComponent(Entity e, ComponentType c) {
       positionCM.add(e, c);
    }

    void World::printDebug() {
        for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
            PositionComponent pos = positionCM.lookup(players[i]);
            std::printf("ECS position x(%f) y(%f) z(%f)\n", pos.position.x, pos.position.y, pos.position.z);
        }
    }

    void World::movePlayer(unsigned int player, float x, float y, float z) {
        // Check x, y, z for valid movement
        PositionComponent& pos = positionCM.lookup(players[player]);
        pos.position = glm::vec3(x,y,z);
    }

} 