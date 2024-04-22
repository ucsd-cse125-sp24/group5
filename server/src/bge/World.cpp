#include "bge/World.h"

namespace bge {

    void World::init() {
        positionCM = ComponentManager<PositionComponent>();
        velocityCM = ComponentManager<VelocityComponent>();
        movementRequestCM = ComponentManager<MovementRequestComponent>();
        std::unique_ptr<MovementSystem> movementSystem = std::make_unique<MovementSystem>();
        std::unique_ptr<PlayerAccelerationSystem> playerAccSystem = std::make_unique<PlayerAccelerationSystem>();
        for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
            Entity newPlayer = createEntity();
            players[i] = newPlayer;
            
            // Create components
            PositionComponent pos = PositionComponent(i*10.0f, 3.0f, -(i%2)*8.0f);
            addComponent(newPlayer, pos);
            VelocityComponent vel = VelocityComponent(0.0f, 0.0f, 0.0f);
            addComponent(newPlayer, vel);
            MovementRequestComponent req = MovementRequestComponent(false, false, false, false, false, 1, 0, 0);
            addComponent(newPlayer, req);

            // Add to systems
            movementSystem->registerEntity(newPlayer);
            playerAccSystem->registerEntity(newPlayer);
        }
        systems.push_back(std::move(movementSystem));
        systems.push_back(std::move(playerAccSystem));
    }

    Entity World::createEntity() {
        Entity newEntity = Entity();
        newEntity.id = currMaxEntityId;
        currMaxEntityId++;
        entities.insert(newEntity);
        return newEntity;
    }

    void World::addComponent(Entity e, PositionComponent c) {
        positionCM.add(e, c);
    }
    void World::addComponent(Entity e, VelocityComponent c) {
        velocityCM.add(e, c);
    }
    void World::addComponent(Entity e, MovementRequestComponent c) {
        movementRequestCM.add(e, c);
    }

    template<typename ComponentType>
    void World::deleteComponent(Entity e, ComponentType c) {
        // Hopefully this actually prints out useful information, apparently the behavior of name() depends on the implementation
        // and may not be human readable
        std::cout << "Error: no remove component function for this component type: " << typeid(ComponentType).name() << std::endl;
        assert(false);
    }
    template<>
    void World::deleteComponent(Entity e, PositionComponent c) {
        positionCM.remove(e);
    }
    template<>
    void World::deleteComponent(Entity e, VelocityComponent c) {
        velocityCM.remove(e);
    }

    void World::updateAllSystems() {
        // this needs to be a reference beause the elements in systems are unique_ptrs
        for (auto& s : systems) {
            s->update();
        }
    }

    void World::printDebug() {
        // TODO: update this

        /* for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
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
        }*/
    }

    void World::movePlayer(unsigned int player, float x, float y, float z) {
        PositionComponent& pos = positionCM.lookup(players[player]);
        pos.position = glm::vec3(x,y,z);
    }

} 