#include "bge/System.h"

namespace bge {

    void System::init() {
    }

    void System::update() {
    }

    void System::registerEntity(Entity entity) {
        registeredEntities.insert(entity);
    }

    void System::deRegisterEntity(Entity entity) {
        auto it = registeredEntities.find(entity);
        if (it != registeredEntities.end()) {
            registeredEntities.erase(it);
        }
    }

    MovementSystem::MovementSystem(std::shared_ptr<ComponentManager<PositionComponent>> positionComponentManager, std::shared_ptr<ComponentManager<VelocityComponent>> velocityComponentManager) {
        positionCM = positionComponentManager;
        velocityCM = velocityComponentManager;
    }

    void MovementSystem::update() {
        for (Entity e : registeredEntities) {
            PositionComponent& pos = positionCM->lookup(e);
            VelocityComponent& vel = velocityCM->lookup(e);
            pos.position += vel.velocity;
        }
    }

    PlayerAccelerationSystem::PlayerAccelerationSystem(std::shared_ptr<ComponentManager<PositionComponent>> positionComponentManager, std::shared_ptr<ComponentManager<VelocityComponent>> velocityComponentManager, std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestComponentManager, std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoComponentManager) {
        positionCM = positionComponentManager;
        velocityCM = velocityComponentManager;
        movementRequestCM = movementRequestComponentManager;
        jumpInfoCM = jumpInfoComponentManager;
    }

    void PlayerAccelerationSystem::update() {
        for (Entity e : registeredEntities) {
            PositionComponent& pos = positionCM->lookup(e);
            VelocityComponent& vel = velocityCM->lookup(e);
            MovementRequestComponent& req = movementRequestCM->lookup(e);
            JumpInfoComponent& jump = jumpInfoCM->lookup(e);

            glm::vec3 rightwardDirection = glm::normalize(glm::cross(req.forwardDirection, glm::vec3(0, 1, 0)));
            glm::vec3 totalDirection = glm::vec3(0);
            float air_modifier = (pos.position.y <= 0.0f) ? 1 : 0.6;

            if (req.forwardRequested)      totalDirection += req.forwardDirection;
            if (req.backwardRequested)     totalDirection -= req.forwardDirection;
            if (req.leftRequested)     totalDirection -= rightwardDirection;
            if (req.rightRequested)    totalDirection += rightwardDirection;

            if (totalDirection != glm::vec3(0)) totalDirection = glm::normalize(totalDirection);

            vel.velocity += totalDirection * MOVEMENT_SPEED * air_modifier;

            if (pos.position.y <= 0.0f) {
                vel.velocity.x *= GROUND_FRICTION;
                vel.velocity.z *= GROUND_FRICTION;
            }
            else {
                vel.velocity.x *= AIR_FRICTION;
                vel.velocity.z *= AIR_FRICTION;
            }
            // Update velocity with accelerations (gravity, player jumping, etc.)
            vel.velocity.y -= jump.jumpHeld ? GRAVITY : GRAVITY * FASTFALL_INCREASE;

            if (jump.jumpHeld && !req.jumpRequested) {
                jump.jumpHeld = false;
            }

            if (!jump.jumpHeld && req.jumpRequested && jump.doubleJumpUsed < MAX_JUMPS_ALLOWED) {
                jump.doubleJumpUsed++;
                vel.velocity.y = JUMP_SPEED;     // as god of physics, i endorse = and not += here
                jump.jumpHeld = true;
            }
        }
    }

    CollisionSystem::CollisionSystem(std::shared_ptr<ComponentManager<PositionComponent>> positionComponentManager, std::shared_ptr<ComponentManager<VelocityComponent>> velocityComponentManager, std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoComponentManager) {
        positionCM = positionComponentManager;
        velocityCM = velocityComponentManager;
        jumpInfoCM = jumpInfoComponentManager;
    }

    void CollisionSystem::update() {
        // Currently only ground collision
        for (Entity e : registeredEntities) {
            PositionComponent& pos = positionCM->lookup(e);
            VelocityComponent& vel = velocityCM->lookup(e);
            JumpInfoComponent& jump = jumpInfoCM->lookup(e);
            // Simple physics: don't fall below the map (assume y=0 now; will change once we have map elevation data / collision boxes)
            if (pos.position.y <= 0.0f) {
                // reset jump states
                pos.position.y = 0.0f;
                vel.velocity.y = 0.0f;
                jump.doubleJumpUsed = 0;
            }
        }
    }
    
}