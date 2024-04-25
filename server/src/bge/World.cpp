#include "bge/World.h"

namespace bge {

    void World::init() {
        // First entity will get index 0
        currMaxEntityId = 0;

        positionCM = std::make_shared<ComponentManager<PositionComponent>>();
        velocityCM = std::make_shared<ComponentManager<VelocityComponent>>();
        movementRequestCM = std::make_shared<ComponentManager<MovementRequestComponent>>();
        jumpInfoCM = std::make_shared<ComponentManager<JumpInfoComponent>>();

        std::cout << Component<PositionComponent>::family() << std::endl;
        std::cout << Component<VelocityComponent>::family() << std::endl;
        std::cout << Component<MovementRequestComponent>::family() << std::endl;
        std::cout << Component<JumpInfoComponent>::family() << std::endl;
        std::cout << Component<PositionComponent>::family() << std::endl;
        std::cout << Component<VelocityComponent>::family() << std::endl;

        std::shared_ptr<PlayerAccelerationSystem> playerAccSystem = std::make_shared<PlayerAccelerationSystem>(positionCM, velocityCM, movementRequestCM, jumpInfoCM);
        std::shared_ptr<MovementSystem> movementSystem = std::make_shared<MovementSystem>(positionCM, velocityCM);
        std::shared_ptr<CollisionSystem> collisionSystem = std::make_shared<CollisionSystem>(positionCM, velocityCM, jumpInfoCM);

        systems.push_back(playerAccSystem);
        systems.push_back(movementSystem);
        systems.push_back(collisionSystem);
    }

    Entity World::createPlayer(int playerID) {

        if (playerID < NUM_PLAYER_ENTITIES) {
            Entity newPlayer = createEntity();
            players[playerID] = newPlayer;
            
            // Create components
            PositionComponent pos = PositionComponent(playerID*10.0f, 3.0f, -(playerID%2)*8.0f);
            addComponent(newPlayer, pos);

            VelocityComponent vel = VelocityComponent(0.0f, 0.0f, 0.0f);
            addComponent(newPlayer, vel);
            MovementRequestComponent req = MovementRequestComponent(false, false, false, false, false, 0, 0);
            addComponent(newPlayer, req);
            JumpInfoComponent jump = JumpInfoComponent(0, false);
            addComponent(newPlayer, jump);

            // Add to systems
            // playerAccSystem->registerEntity(newPlayer);
            // movementSystem->registerEntity(newPlayer);
            // collisionSystem->registerEntity(newPlayer);
        }

    }

    Entity World::createEntity() {
        Entity newEntity = Entity();
        newEntity.id = currMaxEntityId;
        currMaxEntityId++;
        entities.insert(newEntity);
        return newEntity;
    }

    void World::addComponent(Entity e, PositionComponent c) {
        positionCM->add(e, c);
    }
    void World::addComponent(Entity e, VelocityComponent c) {
        velocityCM->add(e, c);
    }
    void World::addComponent(Entity e, MovementRequestComponent c) {
        movementRequestCM->add(e, c);
    }
    void World::addComponent(Entity e, JumpInfoComponent c) {
        jumpInfoCM->add(e, c);
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
        positionCM->remove(e);
    }
    template<>
    void World::deleteComponent(Entity e, VelocityComponent c) {
        velocityCM->remove(e);
    }
    template<>
    void World::deleteComponent(Entity e, MovementRequestComponent c) {
        movementRequestCM->remove(e);
    }
    template<>
    void World::deleteComponent(Entity e, JumpInfoComponent c) {
        jumpInfoCM->remove(e);
    }

    void World::updateAllSystems() {
        // this needs to be a reference beause the elements in systems are unique_ptrs
        for (auto& s : systems) {
            s->update();
        }
    }

    void World::printDebug() {
    }

    void World::updatePlayerInput(unsigned int player, float pitch, float yaw, bool forwardRequested, bool backwardRequested, bool leftRequested, bool rightRequested, bool jumpRequested) {
        MovementRequestComponent& req = movementRequestCM->lookup(players[player]);

        req.pitch = pitch;
        req.yaw = yaw;
        req.forwardRequested = forwardRequested;
        req.backwardRequested = backwardRequested;
        req.leftRequested = leftRequested;
        req.rightRequested = rightRequested;
        req.jumpRequested = jumpRequested;
    }

    Entity World::createProjectile() {

        Entity newProjectile = createEntity();

        PositionComponent pos = PositionComponent(0.0f, 0.0f, 0.0f);
        addComponent(newProjectile, pos);

        VelocityComponent vel = VelocityComponent(0.0f, 0.0f, 0.0f);
        addComponent(newProjectile, vel);

        return newProjectile;

    }

    void World::fillInGameData(ServerToClientPacket& packet) {
        std::vector<PositionComponent> positions = positionCM->getAllComponents();
        for (int i = 0; i < positions.size(); i++) {
            packet.positions[i] = positions[i].position;
        }
        std::vector<VelocityComponent> velocities = velocityCM->getAllComponents();
        for (int i = 0; i < velocities.size(); i++) {
            packet.velocities[i] = velocities[i].velocity;
        }
        std::vector<MovementRequestComponent> requests = movementRequestCM->getAllComponents();
        for (int i = 0; i < requests.size(); i++) {
            packet.pitches[i] = requests[i].pitch;
            packet.yaws[i] = requests[i].yaw;
        }
    }

} 