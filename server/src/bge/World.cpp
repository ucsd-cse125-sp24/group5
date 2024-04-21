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
        // TODO: given a vector of ComponentManagers, add to CM corresponding to c
        positionCM.add(e, c);
    }

    template<typename ComponentType>
    void World::deleteComponent(Entity e, ComponentType c) {
        // TODO: given a vector of ComponentManagers, remove from CM corresponding to c
        positionCM.remove(e);
    }

    void World::printDebug() {
        for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
            PositionComponent pos = positionCM.lookup(players[i]);
            std::printf("Initial ECS position x(%f) y(%f) z(%f)\n", pos.position.x, pos.position.y, pos.position.z);
        }

        Entity testEntity = createEntity();
        PositionComponent pos = PositionComponent(-100,-100,-100);
        positionCM.add(testEntity, pos);
        Entity testEntity2 = createEntity();
        PositionComponent pos2 = PositionComponent(-2,-2,-2);
        positionCM.add(testEntity2, pos2);
        Entity testEntity3 = createEntity();
        PositionComponent pos3 = PositionComponent(-3,-3,-3);
        positionCM.add(testEntity3, pos3);
        std::vector<PositionComponent>& allPos = positionCM.getAllComponents();
        for (int i = 0; i < allPos.size(); i++) {
            PositionComponent pos = allPos[i];
            std::cout << i << std::endl;
            std::printf("ECS position x(%f) y(%f) z(%f)\n", pos.position.x, pos.position.y, pos.position.z);
        }
        
        positionCM.remove(testEntity);
        allPos = positionCM.getAllComponents();
        for (int i = 0; i < allPos.size(); i++) {
            PositionComponent pos = allPos[i];            
            std::cout << i << std::endl;
            std::printf("ECS position x(%f) y(%f) z(%f)\n", pos.position.x, pos.position.y, pos.position.z);
        }
    }

    void World::movePlayer(unsigned int player, float x, float y, float z) {
        // Check x, y, z for valid movement
        PositionComponent& pos = positionCM.lookup(players[player]);
        pos.position = glm::vec3(x,y,z);
    }

} 