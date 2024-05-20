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

	void System::addEventHandler(std::shared_ptr<EventHandler> handler) {
		eventHandlers.push_back(handler);
	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------


	BoxCollisionSystem::BoxCollisionSystem(
        World* gameWorld,
		std::shared_ptr<ComponentManager<PositionComponent>> positionCompManager,
		std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCompManager,
		std::shared_ptr<ComponentManager<BoxDimensionComponent>> dimensionCompManager) {

		world = gameWorld;
        positionCM = positionCompManager;
		eggHolderCM = eggHolderCompManager;
		boxDimensionCM = dimensionCompManager;
        
        std::shared_ptr<EggVsPlayerHandler> eggVsPlayerHandler = std::make_shared<EggVsPlayerHandler>(positionCM, eggHolderCM);
        std::shared_ptr<PlayerStackingHandler> playerStackingHandler = std::make_shared<PlayerStackingHandler>(positionCM, world->velocityCM, world->jumpInfoCM);
        addEventHandler(eggVsPlayerHandler);
        addEventHandler(playerStackingHandler);
	}

	void BoxCollisionSystem::update() {
		// loop through all pairs (no duplicate) and check if there is box collision or not

		for (auto it1 = registeredEntities.begin(); it1 != registeredEntities.end(); ++it1) {
			for (auto it2 = std::next(it1); it2 != registeredEntities.end(); ++it2) {
				Entity ent1 = *it1;
				Entity ent2 = *it2;
				PositionComponent& pos1 = positionCM->lookup(ent1);
				PositionComponent& pos2 = positionCM->lookup(ent2);
				BoxDimensionComponent& dim1 = boxDimensionCM->lookup(ent1);
				BoxDimensionComponent& dim2 = boxDimensionCM->lookup(ent2);
				
				glm::vec3 min1 = pos1.position - dim1.halfDimension;
				glm::vec3 max1 = pos1.position + dim1.halfDimension;
				glm::vec3 min2 = pos2.position - dim2.halfDimension;
				glm::vec3 max2 = pos2.position + dim2.halfDimension;

				bool xOverlap = min1.x <= max2.x && max1.x >= min2.x;
				bool yOverlap = min1.y <= max2.y && max1.y >= min2.y;
				bool zOverlap = min1.z <= max2.z && max1.z >= min2.z;

				if (!xOverlap || !yOverlap || !zOverlap) {
					// no collision
					continue;
				}

				// The entities overlap on all 3 axies, so there is a collision
				// but among which axis did it mainly happened? the one with min (overlap distance / box size)
				float xOverlapDistance = std::min(max1.x - min2.x, max2.x - min1.x);
				float yOverlapDistance = std::min(max1.y - min2.y, max2.y - min1.y);
				float zOverlapDistance = std::min(max1.z - min2.z, max2.z - min1.z);

                float xOverlapRatio = xOverlapDistance / PLAYER_X_WIDTH;
                float yOverlapRatio = yOverlapDistance / PLAYER_Y_HEIGHT;
                float zOverlapRatio = zOverlapDistance / PLAYER_Z_WIDTH;
                float minOverlapRatio = std::min({xOverlapRatio, yOverlapRatio, zOverlapRatio});

				bool is_top_down_collision = (yOverlapRatio == minOverlapRatio);
				// if (is_top_down_collision) {
				// 	std::printf("top down collision detected between entity %d and %d\n", ent1.id, ent2.id); //test
				// } 
				// else {
				// 	std::printf("side to side collision detected between entity %d and %d\n", ent1.id, ent2.id); //test
				// }
					
				for (std::shared_ptr<EventHandler> handler : eventHandlers) {
					// handler->insertPair(ent1, ent2);
					handler->handleInteractionWithData(ent1, ent2, is_top_down_collision, yOverlapDistance);
				}
			}
		}

	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------


	EggMovementSystem::EggMovementSystem(World* gameWorld,
                                         std::shared_ptr<ComponentManager<PositionComponent>> positionCompManager, 
										 std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCompManager,
										 std::shared_ptr<ComponentManager<MovementRequestComponent>> playerRequestCompManager,
										 std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataCompManager) {
		world = gameWorld;
        positionCM = positionCompManager;
		eggHolderCM = eggHolderCompManager;
		moveReqCM = playerRequestCompManager;
		playerDataCM = playerDataCompManager;
	}

	void EggMovementSystem::update() {
		Entity egg = *registeredEntities.begin();
		EggHolderComponent& eggHolder = eggHolderCM->lookup(egg);
        PositionComponent& eggPos = positionCM->lookup(egg);
        VelocityComponent& eggVel = world->velocityCM->lookup(egg);

		if (eggHolder.holderId >= 0) {
            // Egg has owner, follow its movement
			Entity holder = Entity(eggHolder.holderId);
			MovementRequestComponent& req = moveReqCM->lookup(holder);

            // // Drop egg (no throw)?
            // if (req.pitch < -80.0f) { 
            //     // disable egg following
            //     eggHolder.holderId = INT_MIN; 
            // }
            
            // Throw egg?
            if (req.throwEggRequested) {
                // disable egg following
                eggHolder.throwerId = eggHolder.holderId;
                eggHolder.holderId = INT_MIN; 
                eggHolder.isThrown = true;
                // throw egg in the camera's direction + up
                CameraComponent& camera = world->cameraCM->lookup(holder);
                eggVel.velocity += glm::normalize(camera.direction + glm::vec3(0,0.1,0));
                return;
            }

            // Egg follows player
			PositionComponent& holderPos = positionCM->lookup(holder);
			eggPos.position = holderPos.position - req.forwardDirection * EGG_Z_WIDTH;  // egg distance behind player
			PlayerDataComponent& data = playerDataCM->lookup(holder);
			data.points++;
			// if (data.points%3 == 0) {
			// 	printf("Player %d has %d points\n", holder.id, data.points);
			// }
		}
        else {
            // No egg owner. Egg moves by its own velocity
            if (eggVel.onGround) {
                eggVel.velocity.y = 0.0f;
                eggVel.velocity.x *= GROUND_FRICTION;
                eggVel.velocity.z *= GROUND_FRICTION;
            }
            else {
                eggVel.velocity.y -= GRAVITY * 0.8; // how about the egg falls a bit slower :)
            }
            
        }
	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------

    PlayerAccelerationSystem::PlayerAccelerationSystem(
        World* gameWorld,
        std::shared_ptr<ComponentManager<PositionComponent>> positionComponentManager, 
        std::shared_ptr<ComponentManager<VelocityComponent>> velocityComponentManager, 
        std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestComponentManager, 
        std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoComponentManager) {

        world = gameWorld;
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
            req.forwardDirection = forwardDirection;

            glm::vec3 rightwardDirection = glm::cross(forwardDirection, glm::vec3(0, 1, 0));
            req.rightwardDirection = rightwardDirection;
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

	// ------------------------------------------------------------------------------------------------------------------------------------------------

    MovementSystem::MovementSystem(World* gameWorld, std::shared_ptr<ComponentManager<PositionComponent>> positionComponentManager, std::shared_ptr<ComponentManager<MeshCollisionComponent>> meshCollisionComponentManager, std::shared_ptr<ComponentManager<VelocityComponent>> velocityComponentManager) {
        world = gameWorld;
        meshCollisionCM = meshCollisionComponentManager;
        positionCM = positionComponentManager;
        velocityCM = velocityComponentManager;
    }

    void MovementSystem::update() {
        for (Entity e : registeredEntities) {
            if (e.type == EGG && world->eggHolderCM->lookup(e).holderId >= 0) {
                // std::printf("disable egg collision (following player\n");
                continue;
            }

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
                    vel.velocity-=(1-inter.t)*inter.normal*glm::dot(inter.normal, vel.velocity)+0.005f*inter.normal;
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

            if (vel.onGround) {
                if (world->jumpInfoCM->checkExist(e)) {
                    JumpInfoComponent& jump = world->jumpInfoCM->lookup(e);
                    jump.doubleJumpUsed = 0;
                }
            }
        }
    }

	// ------------------------------------------------------------------------------------------------------------------------------------------------

    CameraSystem::CameraSystem(World* _world, std::shared_ptr<ComponentManager<PositionComponent>> _positionCM, std::shared_ptr<ComponentManager<MovementRequestComponent>> _movementRequestCM, std::shared_ptr<ComponentManager<CameraComponent>> _cameraCM) {
        world = _world;
        positionCM = _positionCM;
        movementRequestCM = _movementRequestCM;
        cameraCM = _cameraCM;        
    }

    void CameraSystem::update() {
        for (Entity e : registeredEntities) {

            // if (e.id != 0) return; // remove after testing!! todo
            
            PositionComponent& pos = positionCM->lookup(e);
            MovementRequestComponent& req = movementRequestCM->lookup(e);
            CameraComponent& camera = cameraCM->lookup(e);

            // calculate camera direction (yaw, pitch)
            glm::vec3 direction;
            direction.x = cos(glm::radians(req.yaw)) * cos(glm::radians(req.pitch));
            direction.y = sin(glm::radians(req.pitch));
            direction.z = sin(glm::radians(req.yaw)) * cos(glm::radians(req.pitch));
            direction = glm::normalize(direction);
            camera.direction = direction;

            // shoot ray backwards, determine intersection
            rayIntersection backIntersection = world->intersect(pos.position, -direction, CAMERA_DISTANCE_BEHIND_PLAYER);
            if (backIntersection.t < CAMERA_DISTANCE_BEHIND_PLAYER) {
                // std::printf("detect a mesh that's closer to the player's back than camera is\n");
                // update client with the shorter camera distance 
                camera.distanceBehindPlayer = backIntersection.t;
            }
            else {
                camera.distanceBehindPlayer = CAMERA_DISTANCE_BEHIND_PLAYER;
            }


            
        }

    }

	// ------------------------------------------------------------------------------------------------------------------------------------------------

    BulletSystem::BulletSystem(World* _world, std::shared_ptr<ComponentManager<PositionComponent>> _positionCM, std::shared_ptr<ComponentManager<MovementRequestComponent>> _movementRequestCM, std::shared_ptr<ComponentManager<CameraComponent>> _cameraCM,
    std::shared_ptr<ComponentManager<PlayerDataComponent>> _playerDataCM, std::shared_ptr<ComponentManager<HealthComponent>> _healthCM) {
        world = _world;
        positionCM = _positionCM;
        movementRequestCM = _movementRequestCM;
        cameraCM = _cameraCM;
        playerDataCM = _playerDataCM;

        std::shared_ptr<BulletVsPlayerHandler> bulletVsPlayerHandler = std::make_shared<BulletVsPlayerHandler>(world, _healthCM,  _positionCM);
        addEventHandler(bulletVsPlayerHandler);
    }

    void BulletSystem::update() {
        for (Entity e : registeredEntities) {

            MovementRequestComponent& req = movementRequestCM->lookup(e);
            if (!req.shootRequested) {
                continue;
            }

            PlayerDataComponent& playerData = playerDataCM->lookup(e);
            if (playerData.shootingCD > 0) {        // wait
                playerData.shootingCD--;
                continue;
            }
            else {                                  // shoot, reset timer
                playerData.shootingCD = SHOOTING_CD_TICKS;
            }
            // double seconds = difftime(time(nullptr),playerData.shootingTimer);
            // if (seconds < SHOOTING_CD_TICKS) {		// wait
            //     return;
            // }
            // else {						// shoot, reset timer
            //     time(&playerData.shootingTimer);
            // }

            PositionComponent& playerPos = positionCM->lookup(e);
            CameraComponent& camera = cameraCM->lookup(e);

            // tps ideal hit point : from camera's view
            glm::vec3 viewPosition = playerPos.position + req.forwardDirection * PLAYER_Z_WIDTH + glm::vec3(0,1,0) * CAMERA_DISTANCE_ABOVE_PLAYER;  // above & in front of player, in line with user's camera
            rayIntersection mapInter = world->intersect(viewPosition, camera.direction, BULLET_MAX_T);
            rayIntersection playerInter = world->intersectRayBox(viewPosition, camera.direction, BULLET_MAX_T);
            glm::vec3 idealHitPoint = viewPosition + camera.direction * std::min({mapInter.t, playerInter.t, BULLET_MAX_T});
            
            // shoot another ray from player's gun towards the ideal hit point (matthew's idea)
            // whatever it hits is our real hitPoint. 
            glm::vec3 gunPosition = playerPos.position + req.forwardDirection * PLAYER_Z_WIDTH*1.4f + req.rightwardDirection * PLAYER_Z_WIDTH/2.5f;
            glm::vec3 shootDirection = glm::normalize(idealHitPoint - gunPosition);
            playerInter = world->intersectRayBox(gunPosition, shootDirection, BULLET_MAX_T);
            mapInter = world->intersect(gunPosition, shootDirection, BULLET_MAX_T);
            glm::vec3 hitPoint = gunPosition + shootDirection * std::min({playerInter.t, mapInter.t, BULLET_MAX_T});

            if (playerInter.ent.id == -1 || mapInter.t < playerInter.t) {
                // no player hit
                playerInter.ent.id = -1;
            }
            else {
                // std::printf("bullet ray hits player %d at point x(%f) y(%f) z(%f), rayLength(%f)\n", playerInter.ent.id, hitPoint.x, hitPoint.y, hitPoint[2], playerInter.t); // i just learned that vec[2] is vec.z, amazing
                // use eventhandlers to deal damage
                for (std::shared_ptr<EventHandler> handler : eventHandlers) {
					// handler->insertPair(ent1, ent2);
					handler->handleInteraction(e, playerInter.ent);
				}
            }
            
            world->bulletTrails.push_back({e.id ,gunPosition, hitPoint, playerInter.ent.id});
            // std::printf("push bullet trail gun(%f,%f,%f) -> hit(%f,%f,%f)\n", gunPosition.x, gunPosition.y, gunPosition.z, hitPoint.x, hitPoint.y, hitPoint.z);
        
        }
    }

	// ------------------------------------------------------------------------------------------------------------------------------------------------

    // CollisionSystem::CollisionSystem(World* gameWorld, std::shared_ptr<ComponentManager<PositionComponent>> positionComponentManager, std::shared_ptr<ComponentManager<VelocityComponent>> velocityComponentManager, std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoComponentManager) {
    //     world = gameWorld;
    //     positionCM = positionComponentManager;
    //     velocityCM = velocityComponentManager;
    //     jumpInfoCM = jumpInfoComponentManager;
    // }

    // void CollisionSystem::update() {
    //     // Currently only ground collision
    //     for (Entity e : registeredEntities) {
    //         PositionComponent& pos = positionCM->lookup(e);
    //         VelocityComponent& vel = velocityCM->lookup(e);
    //         JumpInfoComponent& jump = jumpInfoCM->lookup(e);
    //         // Simple physics: don't fall below the map (assume y=0 now; will change once we have map elevation data / collision boxes)
    //         if (vel.onGround) {
    //             jump.doubleJumpUsed = 0;
    //         }
    //     }
    // }
    
}