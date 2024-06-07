#include "bge/System.h"

namespace bge {

    std::mt19937 generator(time(nullptr));
    std::uniform_int_distribution<std::mt19937::result_type> dist;

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

    size_t System::size() {
        return registeredEntities.size();
    }

	// ------------------------------------------------------------------------------------------------------------------------------------------------


	BoxCollisionSystem::BoxCollisionSystem(
        World* gameWorld,
		std::shared_ptr<ComponentManager<PositionComponent>> positionCompManager,
		std::shared_ptr<ComponentManager<EggInfoComponent>> eggInfoCompManager,
		std::shared_ptr<ComponentManager<BoxDimensionComponent>> dimensionCompManager) {

		world = gameWorld;
        positionCM = positionCompManager;
		eggInfoCM = eggInfoCompManager;
		boxDimensionCM = dimensionCompManager;
        
        std::shared_ptr<ProjectileVsPlayerHandler> projectileVsPlayerHandler = std::make_shared<ProjectileVsPlayerHandler>(world->ballProjDataCM);
        std::shared_ptr<EggVsPlayerHandler> eggVsPlayerHandler = std::make_shared<EggVsPlayerHandler>(positionCM, eggInfoCM);
        std::shared_ptr<PlayerStackingHandler> playerStackingHandler = std::make_shared<PlayerStackingHandler>(positionCM, world->velocityCM, world->jumpInfoCM);
        addEventHandler(projectileVsPlayerHandler);
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

                // No collision while lerping
                if (pos1.isLerping || pos2.isLerping) {
                    continue;
                }

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
										 std::shared_ptr<ComponentManager<EggInfoComponent>> eggInfoCompManager,
										 std::shared_ptr<ComponentManager<MovementRequestComponent>> playerRequestCompManager,
										 std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataCompManager) {
		world = gameWorld;
        positionCM = positionCompManager;
		eggInfoCM = eggInfoCompManager;
		moveReqCM = playerRequestCompManager;
		playerDataCM = playerDataCompManager;
	}

	void EggMovementSystem::update() {
		Entity egg = *registeredEntities.begin();
		EggInfoComponent& eggInfo = eggInfoCM->lookup(egg);
        PositionComponent& eggPos = positionCM->lookup(egg);
        VelocityComponent& eggVel = world->velocityCM->lookup(egg);

        // No collision while lerping
        if (eggPos.isLerping) {
            eggVel.velocity = glm::vec3(0);
            return;
        }

		if (eggInfo.holderId >= 0) {
            // Egg has owner, follow its movement
			Entity holder = Entity(eggInfo.holderId);
			MovementRequestComponent& req = moveReqCM->lookup(holder);

            // // Drop egg (no throw)?
            // if (req.pitch < -80.0f) { 
            //     // disable egg following
            //     eggInfo.holderId = INT_MIN; 
            // }
            
            // Throw egg?
            if (req.throwEggRequested) {
                // disable egg following
                eggInfo.throwerId = eggInfo.holderId;
                eggInfo.holderId = INT_MIN; 
                eggInfo.isThrown = true;

			    time(&eggInfo.throwTimer);
                
                // throw egg in the camera's direction + up
                CameraComponent& camera = world->cameraCM->lookup(holder);
                eggPos.position += glm::vec3(0,2,0);        // avoid egg clipped into the map slope while you throw
                eggVel.velocity += glm::normalize(camera.direction + glm::vec3(0,0.1,0));
                eggVel.onGround = false;

                // [if dancebomb] - start detonation timer
                if (eggInfo.eggIsDancebomb) {
                    eggInfo.bombIsThrown = true;
                    eggInfo.detonationTicks = DANCE_BOMB_DENOTATION_TICKS_THROWN;
                }

                return;
            }

            // Egg follows player
			PositionComponent& holderPos = positionCM->lookup(holder);
			eggPos.position = holderPos.position - req.forwardDirection * EGG_Z_WIDTH;  // egg distance behind player
			
            // Holding egg gains you points. But if the egg turns into dancebomb, then don't increase points++,
            // this would discourage gatekeeping the dancebomb --- just throw it. Making players dance would give you more points.
            if (! eggInfo.eggIsDancebomb) {
                PlayerDataComponent& data = playerDataCM->lookup(holder);
                data.points++;
            }
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
        std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoComponentManager,
        std::shared_ptr<ComponentManager<StatusEffectsComponent>> statusEffectsComponentManager) {

        world = gameWorld;
        positionCM = positionComponentManager;
        velocityCM = velocityComponentManager;
        movementRequestCM = movementRequestComponentManager;
        jumpInfoCM = jumpInfoComponentManager;
        statusEffectsCM = statusEffectsComponentManager;
    }

    void PlayerAccelerationSystem::update() {
        for (Entity e : registeredEntities) {
            PositionComponent& pos = positionCM->lookup(e);
            VelocityComponent& vel = velocityCM->lookup(e);

            // No collision while lerping
            if (pos.isLerping) {
                continue;
            }
            
            MovementRequestComponent& req = movementRequestCM->lookup(e);
            JumpInfoComponent& jump = jumpInfoCM->lookup(e);
            StatusEffectsComponent& statusEffects = statusEffectsCM->lookup(e);

            // Dance bomb: disable WASD and attacks
            // experiment: maybe keep jump and ability? 
            if (pos.isBombDancing) {
                req.forwardRequested = req.backwardRequested = req.leftRequested = req.rightRequested = req.shootRequested = false;
            }
            if (req.resetRequested) {
                // This relies on the players having entity ids 0, 1, 2, and 3, 
                // but the alternative is adding another field to the playerData component, initializing it, making this system take that component, etc 
                // That did not seem worth the trouble so we do this instead
                world->resetPlayer(e.id);
                world->resetEgg(e.id);
                continue;
            }

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

            if (statusEffects.swappedControlsTicksLeft > 0) {
                forwardDirection = -forwardDirection;
                rightwardDirection = -rightwardDirection;
                statusEffects.swappedControlsTicksLeft--;
            }

            if (req.forwardRequested)      totalDirection += forwardDirection;
            if (req.backwardRequested)     totalDirection -= forwardDirection;
            if (req.leftRequested)     totalDirection -= rightwardDirection;
            if (req.rightRequested)    totalDirection += rightwardDirection;

            if (totalDirection != glm::vec3(0)) totalDirection = glm::normalize(totalDirection);

            float currentSpeed = MOVEMENT_SPEED;
            if (statusEffects.movementSpeedTicksLeft > 0) {
                currentSpeed = statusEffects.alternateMovementSpeed;
                statusEffects.movementSpeedTicksLeft--;
            }

            vel.velocity += totalDirection * currentSpeed * air_modifier;

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

            if (!jump.jumpHeld && req.jumpRequested && jump.doubleJumpUsed < (world->currentSeason==SUMMER_SEASON?MAX_JUMPS_ALLOWED+1:MAX_JUMPS_ALLOWED)) {
                jump.doubleJumpUsed++;
                float jumpMult=(world->currentSeason==SUMMER_SEASON)?1.25:1; // higher jump height in summer
                if (statusEffects.movementSpeedTicksLeft > 0) {
                    vel.velocity.y = jumpMult*JUMP_SPEED/2; // decrease jump when slowed
                } else vel.velocity.y = jumpMult*JUMP_SPEED;     // as god of physics, i endorse = and not += here
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
            if (e.type == EGG && world->eggInfoCM->lookup(e).holderId >= 0) {
                // std::printf("disable egg collision (following player\n");
                continue;
            }

            PositionComponent& pos = positionCM->lookup(e);
            // No collision while lerping
            if (pos.isLerping) {
                continue;
            }

            VelocityComponent& vel = velocityCM->lookup(e);
            MeshCollisionComponent& meshCol = meshCollisionCM->lookup(e);

            if (meshCol.active) {
                vel.onGround = false;
                glm::vec3 rightDir = glm::cross(vel.velocity, glm::vec3(0, 1, 0));
                glm::vec3 upDir = glm::cross(rightDir, vel.velocity);
                rayIntersection inter;
                int count = 0;
                do {
                    int pointOfInter = -1;
                    inter.t = INFINITY;
                    for (int i = 0; i < meshCol.collisionPoints.size(); i++) {
                        glm::vec3 p0 = pos.position + meshCol.collisionPoints[i];
                        // the t value that is returned is between 0 and 1; it is looking
                        // for a collision between p0+0*vel.velocity and p0+1*vel.velocity
                        rayIntersection newInter = world->intersect(p0, vel.velocity, 1);
                        if (newInter.t < inter.t) {
                            pointOfInter = i;
                            inter = newInter;
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
                } while (inter.t < 1);
            }

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

    SeasonAbilitySystem::SeasonAbilitySystem(World* gameWorld,
        std::shared_ptr<ComponentManager<MovementRequestComponent>> playerRequestComponentManager,
        std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataComponentManager,
        std::shared_ptr<ComponentManager<SeasonAbilityStatusComponent>> seasonAbilityStatusComponentManager,
        std::shared_ptr<ComponentManager<BallProjDataComponent>> ballProjDataComponentManager,
        std::shared_ptr<ComponentManager<PositionComponent>> positionComponentManager,
        std::shared_ptr<ComponentManager<VelocityComponent>> velocityComponentManager,
        std::shared_ptr<ComponentManager<CameraComponent>> cameraComponentManager) {
        world = gameWorld;
        moveReqCM = playerRequestComponentManager;
        playerDataCM = playerDataComponentManager;
        seasonAbilityStatusCM = seasonAbilityStatusComponentManager;
        ballProjDataCM = ballProjDataComponentManager;
        positionCM = positionComponentManager;
        velocityCM = velocityComponentManager;
        cameraCM = cameraComponentManager;
    }

    void SeasonAbilitySystem::update() {
        for (Entity playerEntity : registeredEntities) {
            MovementRequestComponent& req = moveReqCM->lookup(playerEntity);
            PlayerDataComponent& playerData = playerDataCM->lookup(playerEntity);
            SeasonAbilityStatusComponent& seasonAbilityStatus = seasonAbilityStatusCM->lookup(playerEntity);
            if (req.abilityRequested && seasonAbilityStatus.coolDown == 0) {
                seasonAbilityStatus.coolDown = SEASON_ABILITY_CD;
                Entity projEntity;
                if (playerData.playerType == WINTER_PLAYER) {
                    projEntity = world->getFreshProjectile(WINTER);
                } else if (playerData.playerType == SPRING_PLAYER) {
                    projEntity = world->getFreshProjectile(SPRING);
                } else if (playerData.playerType == SUMMER_PLAYER) {
                    projEntity = world->getFreshProjectile(SUMMER);
                } else if (playerData.playerType == AUTUMN_PLAYER) {
                    projEntity = world->getFreshProjectile(AUTUMN);
                }
                BallProjDataComponent& projData = ballProjDataCM->lookup(projEntity);
                PositionComponent& projPos = positionCM->lookup(projEntity);
                VelocityComponent& projVel = velocityCM->lookup(projEntity);
                CameraComponent& camera = cameraCM->lookup(playerEntity);
                PositionComponent& playerPos = positionCM->lookup(playerEntity);

                // This is important to know so we can avoid colliding with the player that created the projectile
                projData.creatorId = playerEntity.id;

                // tps ideal hit point : from camera's view
                glm::vec3 viewPosition = playerPos.position + req.forwardDirection * PLAYER_Z_WIDTH + glm::vec3(0, 1, 0) * CAMERA_DISTANCE_ABOVE_PLAYER;  // above & in front of player, in line with user's camera
                rayIntersection mapInter = world->intersect(viewPosition, camera.direction, PROJ_MAX_T);
                rayIntersection playerInter = world->intersectRayBox(viewPosition, camera.direction, PROJ_MAX_T);
                glm::vec3 idealHitPoint = viewPosition + camera.direction * std::min({ mapInter.t, playerInter.t, BULLET_MAX_T });

                // shoot another ray from the player towards the ideal hit point (matthew's idea)
                // whatever it hits is our real hitPoint. 
                glm::vec3 abilityStartPosition = playerPos.position;
                glm::vec3 shootDirection = glm::normalize(idealHitPoint - abilityStartPosition);
                projPos.position = abilityStartPosition;
                projVel.velocity = shootDirection * PROJ_SPEED;
            }
            if (seasonAbilityStatus.coolDown > 0) {
                seasonAbilityStatus.coolDown--;
            }
        }
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------------

    ProjectileStateSystem::ProjectileStateSystem(World* gameWorld,
        std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataComponentManager,
        std::shared_ptr<ComponentManager<StatusEffectsComponent>> statusEffectsComponentManager,
        std::shared_ptr<ComponentManager<BallProjDataComponent>> ballProjDataComponentManager,
        std::shared_ptr<ComponentManager<PositionComponent>> positionComponentManager,
        std::shared_ptr<ComponentManager<VelocityComponent>> velocityComponentManager,
        std::shared_ptr<ComponentManager<MeshCollisionComponent>> meshCollisionComponentManager,
        std::shared_ptr<ComponentManager<HealthComponent>> healthComponentManager) {
        world = gameWorld;
        playerDataCM = playerDataComponentManager;
        statusEffectsCM = statusEffectsComponentManager;
        ballProjDataCM = ballProjDataComponentManager;
        positionCM = positionComponentManager;
        velocityCM = velocityComponentManager;
        meshCollisionCM = meshCollisionComponentManager;
        healthCM = healthComponentManager;
    }

    void ProjectileStateSystem::update() {
        for (Entity e : registeredEntities) {
            VelocityComponent& vel = velocityCM->lookup(e);
            PositionComponent& pos = positionCM->lookup(e);
            BallProjDataComponent& projData = ballProjDataCM->lookup(e);
            if (projData.exploded) {
                // send the projectile away/make it inactive
                projData.active = false;
                projData.collidedWithPlayer = false;
                projData.exploded = false;
                pos.position = world->voidLocation;
                vel.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
            }
            // If this projectile is active and it collided with the map, collided with a player, or left the map, make it explode
            if (projData.active && (vel.onGround || projData.collidedWithPlayer || !(world->withinMapBounds(pos.position)))) {
                // Check if any players are in the radius of the explosion
                for (Entity playerEntity : world->players) {
                    VelocityComponent& playerVel = velocityCM->lookup(playerEntity);
                    PositionComponent& playerPos = positionCM->lookup(playerEntity);
                    StatusEffectsComponent& statusEffects = statusEffectsCM->lookup(playerEntity);
                    HealthComponent& health = healthCM->lookup(playerEntity);
                    float distFromExplosion = glm::distance(playerPos.position, pos.position);
                    distFromExplosion = std::max(0.0f, distFromExplosion - PROJ_EXPLOSION_RADIUS_MAX_EFFECT);
                    if (projData.type == WINTER) {
                        if (distFromExplosion < PROJ_EXPLOSION_RADIUS) {
                            // effect time = (r-x)^2 * mt / (r^2)
                            // where r is the radius of the explosion (max distance away where players are still affected)
                            // x is the distance of this player from the explosion
                            // mt is the maximum amount of time the effect can last
                            // (So the effect will last the maximum amount of time for players who are exactly at the ball and 0 time for players who are exactly at the boundary)
                            statusEffects.movementSpeedTicksLeft = (PROJ_EXPLOSION_RADIUS - distFromExplosion) * (PROJ_EXPLOSION_RADIUS - distFromExplosion) * MAX_PROJ_EFFECT_LENGTH / (PROJ_EXPLOSION_RADIUS * PROJ_EXPLOSION_RADIUS);
                            statusEffects.alternateMovementSpeed = SLOW_MOVEMENT_SPEED;
                        }
                    }
                    else if (projData.type == SPRING) {
                        // We may want to use different constants for explosion radius in the future for different seasons,
                        // so the radius check is separated by season
                        // Health effect only works on you and your teammate
                        /*if (distFromExplosion < PROJ_EXPLOSION_RADIUS) {
                            std::cout << "Player " << playerEntity.id << "was hit! Their teammate is player " << world->teammates[playerEntity.id] << ", and the player who created the projectile is player " << projData.creatorId << std::endl;
                        }
                        else {
                            std::cout << "player " << playerEntity.id << " wasn't hit because they were " << distFromExplosion << " away\n";
                        }*/
                        if (distFromExplosion < PROJ_EXPLOSION_RADIUS && (playerEntity.id == projData.creatorId || world->teammates[playerEntity.id] == projData.creatorId)) {
                            float healStrength = (PROJ_EXPLOSION_RADIUS - distFromExplosion) * (PROJ_EXPLOSION_RADIUS - distFromExplosion) * MAX_HEAL_STRENGTH / (PROJ_EXPLOSION_RADIUS * PROJ_EXPLOSION_RADIUS);
                            std::cout << "Healing " << healStrength << "!\n";
                            health.healthPoint += healStrength;
                            if (health.healthPoint > PLAYER_MAX_HEALTH) {
                                health.healthPoint = PLAYER_MAX_HEALTH;
                            }
                        }
                    }
                    else if (projData.type == SUMMER) {
                        if (distFromExplosion < PROJ_EXPLOSION_RADIUS) {
                            statusEffects.swappedControlsTicksLeft = (PROJ_EXPLOSION_RADIUS - distFromExplosion) * (PROJ_EXPLOSION_RADIUS - distFromExplosion) * MAX_PROJ_EFFECT_LENGTH / (PROJ_EXPLOSION_RADIUS * PROJ_EXPLOSION_RADIUS);
                        }
                    }
                    else if (projData.type == AUTUMN) {
                        if (distFromExplosion < PROJ_EXPLOSION_RADIUS) {
                            glm::vec3 launchDir;
                            if (projData.collidedWithPlayer && projData.collisionPlayerId == playerEntity.id) {
                                // For the player we collided with, just use the projectile's velocity,
                                // since the projectile's location relative to the player's location can be weird
                                launchDir = glm::normalize(vel.velocity);
                            }
                            else {
                                launchDir = glm::normalize(playerPos.position - pos.position);
                            }
                            float launchSeverity = (PROJ_EXPLOSION_RADIUS - distFromExplosion) * (PROJ_EXPLOSION_RADIUS - distFromExplosion) * MAX_LAUNCH_SEVERITY / (PROJ_EXPLOSION_RADIUS * PROJ_EXPLOSION_RADIUS);
                            playerVel.velocity += launchSeverity * launchDir;
                        }
                    }
                }
                projData.exploded = true;
            }
        }
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------------

    SeasonEffectSystem::SeasonEffectSystem(
        World* _gameWorld,
		std::shared_ptr<ComponentManager<HealthComponent>> _healthCM,
		std::shared_ptr<ComponentManager<VelocityComponent>> _velocityCM,
        std::shared_ptr<ComponentManager<MovementRequestComponent>> _movementRequestCM, 
        std::shared_ptr<ComponentManager<JumpInfoComponent>> _jumpInfoCM, 
        std::shared_ptr<ComponentManager<SeasonAbilityStatusComponent>> _seasonAbilityStatusCM) {
        
        world = _gameWorld;
        healthCM = _healthCM;
        velocityCM = _velocityCM;
        movementRequestCM = _movementRequestCM;
        jumpInfoCM = _jumpInfoCM;
        seasonAbilityStatusCM = _seasonAbilityStatusCM;

    }

    void SeasonEffectSystem::update() {

        // Change this to make each season longer or shorter
        if (world->seasonCounter > SEASON_LENGTH) {
            world->seasonCounter = 0;
            world->currentSeason = (world->currentSeason+1)%4;
            // std::printf("Current Season is %d\n", world->currentSeason);
        }

        // For Debugging
        // world->currentSeason = WINTER_SEASON;

        if (world->currentSeason == SPRING_SEASON) {
            for (Entity e : registeredEntities) {
                HealthComponent& health = healthCM->lookup(e);
                if (world->seasonCounter % 50 == 0) {
                    health.healthPoint = std::min(PLAYER_MAX_HEALTH,health.healthPoint+5);
                }
            }
        } else if (world->currentSeason == SUMMER_SEASON) {
            for (Entity e : registeredEntities) {
                JumpInfoComponent& jump = jumpInfoCM->lookup(e);
                MovementRequestComponent& req = movementRequestCM->lookup(e);
                VelocityComponent& vel = velocityCM->lookup(e);

                // if (!jump.jumpHeld && req.jumpRequested && jump.doubleJumpUsed == MAX_JUMPS_ALLOWED) {
                //     jump.doubleJumpUsed++;
                //     vel.velocity.y = JUMP_SPEED*1.25;
                //     jump.jumpHeld = true;
                // }
            }
        } else if (world->currentSeason == AUTUMN_SEASON) {
            for (Entity e : registeredEntities) {
                SeasonAbilityStatusComponent& seasonAbilityStatus = seasonAbilityStatusCM->lookup(e);
                // Reduce cooldown by 66%
                if (seasonAbilityStatus.coolDown < SEASON_ABILITY_CD*2/3) {
                    seasonAbilityStatus.coolDown = 0;
                }
            }
        } else if (world->currentSeason == WINTER_SEASON) {
            for (Entity e : registeredEntities) {
                MovementRequestComponent& req = movementRequestCM->lookup(e);
                VelocityComponent& vel = velocityCM->lookup(e);
                if (vel.onGround) {
                    vel.timeOnGround++;
                    float speedMult = 1 + std::min(vel.timeOnGround, 90) * 0.004;
                    vel.velocity.x *= speedMult;
                    vel.velocity.z *= speedMult;
                } else {
                    vel.timeOnGround = 0;
                    float speedMult = 0.8;
                    vel.velocity.x *= speedMult;
                    vel.velocity.z *= speedMult;
                }
            }
        }

        world->seasonCounter++;

    }

    // ------------------------------------------------------------------------------------------------------------------------------------------------

    LerpingSystem::LerpingSystem(World* _world) {
        world = _world;
    }

    void LerpingSystem::update() {
        for (Entity e : registeredEntities) {
            PositionComponent& pos = world->positionCM->lookup(e);
            if (! pos.isLerping) {
                continue;
            }

            // std::printf("processing learping for entity %d\n", e.id);

            // Apply lerp and update position
            LerpingComponent& lerp = world->lerpingCM->lookup(e);
            lerp.curr += lerp.delta;
            pos.position = lerp.curr;


            // check lerping end condition
            if (--lerp.t <= 0.01) {
                pos.isLerping = false;
                world->deleteComponent(e, lerp);
            }


        }

        // debug check
        // std::printf("number of lerping components = %d\n", world->lerpingCM->getAllComponents().size());
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------------

    DanceBombSystem::DanceBombSystem(World* _world) {
        world = _world;
        // Make sure we don't have dance bombs at the very beginning of the game (too chaotic)
        time_t danceBombsBecomePossible = NO_DANCE_BOMBS_PORTION * GAME_DURATION;
        long long bucketLength = (1 - NO_DANCE_BOMBS_PORTION) * GAME_DURATION / DANCE_BOMBS_PER_GAME;
        for (unsigned int i = 0; i < DANCE_BOMBS_PER_GAME; i++) {
            long long randomOffset = dist(generator) % bucketLength;
            // Dance bomb happens at a random time within this bucket
            danceBombTimes[i] = danceBombsBecomePossible + bucketLength * i + randomOffset;
            std::cout << "Will explode at " << danceBombTimes[i] << std::endl;
        }
    }

    void DanceBombSystem::update() {
        Entity egg = world->getEgg();
        EggInfoComponent& bomb = world->eggInfoCM->lookup(egg);
        PositionComponent& eggPos = world->positionCM->lookup(egg);
        VelocityComponent& eggVel = world->velocityCM->lookup(egg);


        // Stage 0: Dancebomb is still an egg -> turn it into dancebomb on condition
        if (!bomb.eggIsDancebomb) {
            // Hardcoded dancebomb secret key: player 0 presses all four WASD
            MovementRequestComponent& req = world->movementRequestCM->lookup(world->players[0]);

            time_t now = time(nullptr);
            long long timeSinceStart = now - world->worldTimer;
            bool danceBombRequested = false;
            if (nextDanceBomb < DANCE_BOMBS_PER_GAME && danceBombTimes[nextDanceBomb] <= timeSinceStart) {
                danceBombRequested = true;
                nextDanceBomb++;
            }
            danceBombRequested |= req.bombRequested;        // player 0 secret key : B
            if (danceBombRequested) {
                bomb.eggIsDancebomb = true;
                std::printf("egg is dance bomb : %d\n", bomb.eggIsDancebomb);

                // if (!bomb.bombIsThrown && bomb.holderId >= 0) {
                //     bomb.detonationTicks = DANCE_BOMB_DENOTATION_TICKS_HOLD;
                //     world->velocityCM->lookup(egg).onGround = false;
                // }
            }
            // Todo: get rid of hardcode: use this below
            // if (some condition, eg. time reaches 400 ticks), create the dancebomb
            // if (condition) { //todo
            //     bomb.eggIsDancebomb = true;
            // }

            // dance bomb creation
            // 1) make it drop from current player? so he has to pick it up... instead of instantly getting the bomb
            // 2) or ascend the egg into the sky, randomize its xz coordinate, and fall as a dance bomb. ? cooler visual

            else {
                // no point continuing with this system if there is no dance bomb
                return;
            }
        }


        // no point continuing with this system if there is no dance bomb
        if (!bomb.eggIsDancebomb) {
            return;
        }

        if (!bomb.bombIsThrown && bomb.holderId < 0) {
            return;
        }
        // Now, the bomb is either carried a player or is thrown. The count down starts! 
        

        // Stage 1: Dancebomb not in action
        if (!bomb.danceInAction) {

            // countdown detonation timer
            bomb.detonationTicks--;
            // std::printf("dancebomb detonation ticks left: %d\n", bomb.detonationTicks);

            // make bomb stop and explode quick if it hits ground (due to throwing)
            if (world->velocityCM->lookup(egg).onGround && bomb.bombIsThrown) {
                bomb.detonationTicks = std::min(8, bomb.detonationTicks);
            }

            // in case timer reaches 0, explode the bomb and mark surrouding players as isBombDancing
            if (bomb.detonationTicks == 0) {

                // stop bomb movement 
                eggVel.velocity = glm::vec3(0.05, 0.05,0.05);
                
                int numPlayersDancing = 0;
                for (Entity player : world->players) {
                    PositionComponent& playerPos = world->positionCM->lookup(player);

                    if (glm::length(eggPos.position - playerPos.position) <= DANCE_BOMB_RADIUS) {
                        playerPos.isBombDancing = true;
                        numPlayersDancing++;
                        std::printf("dancebomb affects player %d\n", player.id);
                    }
                }
                
                // add points to the dancebomb thrower
                Entity badGuy = (bomb.bombIsThrown) ? bomb.throwerId : bomb.holderId;
                // std::printf("bad guy is player %d\n", badGuy.id);
                PlayerDataComponent& badGuyData = world->playerDataCM->lookup(badGuy);
                badGuyData.points += numPlayersDancing * 10<<5; // plus 10 points per player hit

                bomb.danceInAction = true;      // move to Stage 2: DanceBomb in action
                bomb.danceBombStartTime = time(nullptr);
                std::printf("start dancing!\n");
            }

        }


        // Stage 2: DanceBomb in action
        if (bomb.danceInAction) {

            // keep players dancing (until the dance duration ends) 
            time_t now = time(nullptr);
            time_t dancingTimeSecs = now - bomb.danceBombStartTime;
            // std::printf("[stage2] dancingTimeSecs = %ld\n", dancingTimeSecs);
            // todo: send dancingTimeSecs to client for rendering
            
            if (dancingTimeSecs < DANCE_BOMB_DURATION_SECS) {
                // keep players dancing
                // std::printf("[stage2] players shall dance\n");

            }
            else {
                // end dancebomb, free players
                std::printf("[stage2] end of dance bomb\n");

                // reset egg info
                bomb.bombIsThrown = false;
                bomb.eggIsDancebomb = false;
                bomb.detonationTicks = DANCE_BOMB_DENOTATION_TICKS_HOLD;
                bomb.danceInAction = false;

                // reset players' bomb dance status
                for (Entity player : world->players) {
                    PositionComponent& playerPos = world->positionCM->lookup(player);
                    playerPos.isBombDancing = false;
                }

                // shoot the egg to the sky (so that people get hit won't instantly get the egg)
                bomb.holderId = -1;
                bomb.isThrown = true;

                eggVel.onGround = false;
                eggVel.velocity = glm::vec3(0);
                eggPos.isLerping = true;


                // generate random values
                std::random_device rd;  // Seed
                std::mt19937 gen(rd()); // Standard mersenne_twister_engine

                // Define the range
                std::uniform_int_distribution<> dis(-15, 15);

                // Generate random number
                int random_value_x = dis(gen);
                int random_value_z = dis(gen);


                glm::vec3 eggRespawnPosition = glm::vec3(random_value_x, 18, random_value_z); // above the warren bear :)
                world->addComponent(egg, LerpingComponent(eggPos.position, eggRespawnPosition, 20.0f));

            }
            
        }
    

    }

    GodMovementSystem::GodMovementSystem(World* _world) {
        world = _world;
    }
    
    void GodMovementSystem::update() {
        if (world->godPlayer != INT_MIN) {
            
            Entity godPlayer = world->players[0];
            
            PositionComponent& pos = world->positionCM->lookup(godPlayer);

            MovementRequestComponent& req = world->movementRequestCM->lookup(godPlayer);
            
            if (req.backwardRequested && req.jumpRequested) {
                position.y -= 1;
                req.jumpRequested = false;
                req.backwardRequested = false;
            } 

            if (req.jumpRequested)     position.y += 1;

            glm::vec3 forwardDirection;
            forwardDirection.x = cos(glm::radians(req.yaw));
            forwardDirection.y = 0;
            forwardDirection.z = sin(glm::radians(req.yaw));
            forwardDirection = glm::normalize(forwardDirection);

            glm::vec3 rightwardDirection = glm::cross(forwardDirection, glm::vec3(0, 1, 0));
            req.rightwardDirection = rightwardDirection;
            glm::vec3 totalDirection = glm::vec3(0);

            if (req.forwardRequested)      totalDirection += forwardDirection;
            if (req.backwardRequested)     totalDirection -= forwardDirection;
            if (req.leftRequested)     totalDirection -= rightwardDirection;
            if (req.rightRequested)    totalDirection += rightwardDirection;

            if (totalDirection != glm::vec3(0)) totalDirection = glm::normalize(totalDirection);

            position += totalDirection;
            pos = position;

        } else {
            position = world->positionCM->lookup(world->players[0]).position;
        }

    }

}


