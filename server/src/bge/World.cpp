#include "bge/World.h"

#include <random>

namespace bge {

    // for debug purpose -
    void deleteShit(Entity e) {

    }

    void World::init() {
        // First entity will get index 0
        currMaxEntityId = 0;

        positionCM = std::make_shared<ComponentManager<PositionComponent>>();
        lastPositionCM = std::make_shared<ComponentManager<PositionComponent>>();

        velocityCM = std::make_shared<ComponentManager<VelocityComponent>>();
        jumpInfoCM = std::make_shared<ComponentManager<JumpInfoComponent>>();
        movementRequestCM = std::make_shared<ComponentManager<MovementRequestComponent>>();

        healthCM = std::make_shared<ComponentManager<HealthComponent>>();
        dimensionCM = std::make_shared<ComponentManager<DimensionComponent>>();

        eggHolderCM = std::make_shared<ComponentManager<EggHolderComponent>>();


        std::shared_ptr<PlayerAccelerationSystem> playerAccSystem = std::make_shared<PlayerAccelerationSystem>(positionCM, velocityCM, movementRequestCM, jumpInfoCM);
        std::shared_ptr<MovementSystem> movementSystem = std::make_shared<MovementSystem>(positionCM, velocityCM);
        std::shared_ptr<PlayerVSGroundCollisionSystem> playerVSGroundCollisionSystem = std::make_shared<PlayerVSGroundCollisionSystem>(positionCM, velocityCM, jumpInfoCM);
        std::shared_ptr<BoxCollisionSystem> boxCollisionSystem = std::make_shared<BoxCollisionSystem>(positionCM, eggHolderCM, dimensionCM);
        std::shared_ptr<EggMovementSystem> eggMovementSystem = std::make_shared<EggMovementSystem>(positionCM, eggHolderCM, movementRequestCM);


        // TODO: figure out a way to pass the deleteEntity method to this function
        // right now I have to pass a random function to this
        projectileVsPlayerHandler = std::make_shared<ProjectileVsPlayerHandler>(deleteShit, healthCM);
        eggVsPlayerHandler = std::make_shared<EggVsPlayerHandler>(deleteShit, positionCM, eggHolderCM);

        boxCollisionSystem->addEventHandler(eggVsPlayerHandler);

        // init players
        for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
            Entity newPlayer = createEntity(PLAYER);
            players[i] = newPlayer;
            
            // Create components
            PositionComponent pos = PositionComponent(i*10.0f, 3.0f, -(i%2)*8.0f);
            addComponent(newPlayer, pos);

            VelocityComponent vel = VelocityComponent(0.0f, 0.0f, 0.0f);
            addComponent(newPlayer, vel);
            MovementRequestComponent req = MovementRequestComponent(false, false, false, false, false, 0, 0);
            addComponent(newPlayer, req);
            JumpInfoComponent jump = JumpInfoComponent(0, false);
            addComponent(newPlayer, jump);

            // Add to systems
            playerAccSystem->registerEntity(newPlayer);
            movementSystem->registerEntity(newPlayer);
            playerVSGroundCollisionSystem->registerEntity(newPlayer);
            boxCollisionSystem->registerEntity(newPlayer);

            // add to event handler
            eggVsPlayerHandler->registerEntity(newPlayer);

            // // TODO: create an egg object and add that egg object to eggHolderComponent
            // // for now, probably just use the player 4 as egg
            // if (i == 2) {
            //     EggHolderComponent eggHolder = EggHolderComponent(INT_MIN);
            //     eggMovementSystem->egg = newPlayer;
            //     addComponent(newPlayer, eggHolder);
            // }
        }

        // init egg
        egg = createEntity(EGG);
        PositionComponent pos = PositionComponent(10.0f, 0.0f, 10.0f);
        addComponent(egg, pos);
        EggHolderComponent eggHolder = EggHolderComponent(INT_MIN);
        addComponent(egg, eggHolder);
        eggMovementSystem->registerEntity(egg);
        boxCollisionSystem->registerEntity(egg);

        // init trees, rocks, house's bounding boxes. (todo)


        /* Do Not Change the Order of the Code Above or Below. 
            The order in which these components are created
            is the only way for the server and clients to
            agree on which entity is which 
        */

        systems.push_back(playerAccSystem);
        systems.push_back(movementSystem);
        systems.push_back(playerVSGroundCollisionSystem);
        systems.push_back(boxCollisionSystem);
        systems.push_back(eggMovementSystem);

    }

    Entity World::createEntity(EntityType type) {
        Entity newEntity = Entity();
        newEntity.id = currMaxEntityId;
        newEntity.type = type;
        currMaxEntityId++;
        entities.insert(newEntity);
        return newEntity;
    }

    void World::deleteEntity(Entity entity) {
        // TODO: remove an entity from all system, component manager, etc.
    }

    void World::addComponent(Entity e, PositionComponent c) {
        positionCM->add(e, c);
    }
    void World::addComponent(Entity e, VelocityComponent c) {
        velocityCM->add(e, c);
    }
    void World::addComponent(Entity e, JumpInfoComponent c) {
        jumpInfoCM->add(e, c);
    }
    void World::addComponent(Entity e, MovementRequestComponent c) {
        movementRequestCM->add(e, c);
    }
    void World::addComponent(Entity e, HealthComponent c) {
        healthCM->add(e, c);
    }
    void World::addComponent(Entity e, DimensionComponent c) {
        dimensionCM->add(e, c);   
    }
    void World::addComponent(Entity e, EggHolderComponent c) {
        eggHolderCM->add(e, c);
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
        // this needs to be a reference because the elements in systems are unique_ptrs
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

    void World::createProjectile() {

        Entity newProjectile = createEntity(PROJECTILE);

        PositionComponent pos = PositionComponent(0.0f, 0.0f, 0.0f);
        addComponent(newProjectile, pos);

        VelocityComponent vel = VelocityComponent(0.0f, 0.0f, 0.0f);
        addComponent(newProjectile, vel);

        projectileVsPlayerHandler->registerEntity(newProjectile);
    }

    void World::fillInGameData(ServerToClientPacket& packet) {
        std::vector<PositionComponent> positions = positionCM->getAllComponents();
        for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
            packet.positions[i] = positions[i].position;
        }
        // std::vector<VelocityComponent> velocities = velocityCM->getAllComponents();
        // for (int i = 0; i < velocities.size(); i++) {
        //     packet.velocities[i] = velocities[i].velocity;
        // }
        std::vector<MovementRequestComponent> requests = movementRequestCM->getAllComponents();
        for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
            packet.pitches[i] = requests[i].pitch;
            packet.yaws[i] = requests[i].yaw;
        }
    }

} 