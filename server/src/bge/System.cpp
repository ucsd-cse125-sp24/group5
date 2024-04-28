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

            glm::vec3 forwardDirection;
            forwardDirection.x = cos(glm::radians(req.yaw));
            forwardDirection.y = 0;
            forwardDirection.z = sin(glm::radians(req.yaw));
            forwardDirection = glm::normalize(forwardDirection);

            glm::vec3 rightwardDirection = glm::normalize(glm::cross(forwardDirection, glm::vec3(0, 1, 0)));
            glm::vec3 totalDirection = glm::vec3(0);
            float air_modifier = (pos.position.y <= 0.0f) ? 1 : AIR_MOVEMENT_MODIFIER;

            if (req.forwardRequested)      totalDirection += forwardDirection;
            if (req.backwardRequested)     totalDirection -= forwardDirection;
            if (req.leftRequested)     totalDirection -= rightwardDirection;
            if (req.rightRequested)    totalDirection += rightwardDirection;

            if (totalDirection != glm::vec3(0)) totalDirection = glm::normalize(totalDirection);

            vel.velocity += totalDirection * MOVEMENT_SPEED * air_modifier;

            if (pos.position.y <= -5.0f) {
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

    MovementSystem::MovementSystem(World* gameWorld, std::shared_ptr<ComponentManager<PositionComponent>> positionComponentManager, std::shared_ptr<ComponentManager<VelocityComponent>> velocityComponentManager) {
        world = gameWorld;
        positionCM = positionComponentManager;
        velocityCM = velocityComponentManager;
    }

    void MovementSystem::update() {
        for (Entity e : registeredEntities) {
            PositionComponent& pos = positionCM->lookup(e);
            VelocityComponent& vel = velocityCM->lookup(e);

            float playerSize=1;

            glm::vec3 rightDir=glm::cross(vel.velocity, glm::vec3(0,1,0));
            glm::vec3 upDir=glm::cross(rightDir, vel.velocity);
            rayIntersection inter;
            int count=0;
            do {
                inter.t = INFINITY;
                for(float x=-1; x<=1; x+=1) {
                    for(float y=-1; y<=1; y+=1) {
                        for(float z=-1; z<=1; z+=1) {
                            glm::vec3 p0;
                            if(x==0&&y==0&&z==0) continue;
                            glm::vec3 offset=playerSize*glm::normalize(glm::vec3(x, y, z));
                            offset.y*=2;
                            p0=pos.position+offset;
                            rayIntersection newInter=world->intersect(p0, vel.velocity, 1);
                            if(newInter.t<inter.t) {
                                inter=newInter;
                            }
                        }
                    }
                }
                if(inter.t<1) {
                    vel.velocity-=(1-inter.t)*inter.normal*glm::dot(inter.normal, vel.velocity)+0.01f*inter.normal;
                    count++;
                    if(count==10) break;
                }
            } while(inter.t<1);

            // std::cout<<"number of collisions: "<<count<<std::endl;

            pos.position += vel.velocity;
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
            if (pos.position.y <= -5.0f) {
                // reset jump states
                pos.position.y = -5.0f;
                vel.velocity.y = 0.0f;
                jump.doubleJumpUsed = 0;
            }
            jump.doubleJumpUsed=0;
        }
    }
    
}