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
            float air_modifier = (vel.onGround) ? 1 : AIR_MOVEMENT_MODIFIER;

            if (req.forwardRequested)      totalDirection += forwardDirection;
            if (req.backwardRequested)     totalDirection -= forwardDirection;
            if (req.leftRequested)     totalDirection -= rightwardDirection;
            if (req.rightRequested)    totalDirection += rightwardDirection;

            if (totalDirection != glm::vec3(0)) totalDirection = glm::normalize(totalDirection);

            vel.velocity += totalDirection * MOVEMENT_SPEED * air_modifier;

            if (vel.onGround) {
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

    MovementSystem::MovementSystem(World* gameWorld, std::shared_ptr<ComponentManager<PositionComponent>> positionComponentManager, std::shared_ptr<ComponentManager<MeshCollisionComponent>> meshCollisionComponentManager, std::shared_ptr<ComponentManager<VelocityComponent>> velocityComponentManager) {
        world = gameWorld;
        meshCollisionCM = meshCollisionComponentManager;
        positionCM = positionComponentManager;
        velocityCM = velocityComponentManager;
    }

    void MovementSystem::update() {
        for (Entity e : registeredEntities) {
            PositionComponent& pos = positionCM->lookup(e);
            VelocityComponent& vel = velocityCM->lookup(e);
            MeshCollisionComponent& meshCol = meshCollisionCM->lookup(e);

            vel.onGround=false;
            glm::vec3 rightDir=glm::cross(vel.velocity, glm::vec3(0,1,0));
            glm::vec3 upDir=glm::cross(rightDir, vel.velocity);
            rayIntersection inter;
            int count=0;
            do {
                int pointOfInter=-1;
                inter.t = INFINITY;
                for(int i=0; i<meshCol.collisionPoints.size(); i++) {
                    glm::vec3 p0=pos.position+meshCol.collisionPoints[i];
                    // the t value that is returned is between 0 and 1; it is looking
                    // for a collision between p0+0*vel.velocity and p0+1*vel.velocity
                    rayIntersection newInter=world->intersect(p0, vel.velocity, 1);
                    if(newInter.t<inter.t) {
                        pointOfInter=i;
                        inter=newInter;
                    }
                }
                if(inter.t<1) {
                    bool stationaryOnGround=false;
                    for(int i=0; i<meshCol.groundPoints.size(); i++) {
                        if(meshCol.groundPoints[i]==pointOfInter) {
                            vel.onGround=true;
                            glm::vec3 velHorizontal=glm::vec3(vel.velocity.x, 0, vel.velocity.z);
                            // if there is very low horizontal velocity, stop the player
                            if(length(velHorizontal)<0.05) {
                                stationaryOnGround=true;
                            }
                        }
                    }
                    // remove the velocity in the direction of the triangle except a little bit less
                    // so you aren't fully in the wall
                    vel.velocity-=(1-inter.t)*inter.normal*glm::dot(inter.normal, vel.velocity)+0.01f*inter.normal;
                    if(stationaryOnGround) {
                        vel.velocity=glm::vec3(0);
                    }
                    count++;
                    // we cap it at 100 collisions per second; this is pretty generous
                    if(count==100) break;
                }
            } while(inter.t<1);

            // std::cout<<count<<std::endl;

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
            if (vel.onGround) {
                jump.doubleJumpUsed = 0;
            }
        }
    }
    
}