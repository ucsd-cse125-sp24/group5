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

	// ------------------------------------------------------------------------------------------------------------------------------------------------


	PlayerVSGroundCollisionSystem::PlayerVSGroundCollisionSystem(std::shared_ptr<ComponentManager<PositionComponent>> positionComponentManager, std::shared_ptr<ComponentManager<VelocityComponent>> velocityComponentManager, std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoComponentManager) {
		positionCM = positionComponentManager;
		velocityCM = velocityComponentManager;
		jumpInfoCM = jumpInfoComponentManager;
	}

	void PlayerVSGroundCollisionSystem::update() {
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

	// ------------------------------------------------------------------------------------------------------------------------------------------------



	BoxCollisionSystem::BoxCollisionSystem(
		std::shared_ptr<ComponentManager<PositionComponent>> positionCompManager,
		std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCompManager,
		std::shared_ptr<ComponentManager<DimensionComponent>> dimensionCompManager) {

		positionCM = positionCompManager;
		eggHolderCM = eggHolderCompManager;
		dimensionCM = dimensionCompManager;
	}

	void BoxCollisionSystem::update() {
		// loop through all user position and try to find one that has collision with the egg

		// first, find which one is the egg
		// we are guaranteed that there is only one egg here - at least for now
		Entity egg{};
		for (Entity e : registeredEntities) {
			bool isEgg = eggHolderCM->checkExist(e);
			if (isEgg) {
				egg = e;
				break;
			}
		}

		// then, run through all other components and see which one we collide with egg
		PositionComponent eggPositionComp = positionCM->lookup(egg);
		for (Entity e : registeredEntities) {
			if (egg.id == e.id) continue;

			// TODO: check for collision and if that happen, call for event handler
			PositionComponent playerPositionComp = positionCM->lookup(e);
			auto distance = glm::length(playerPositionComp.position - eggPositionComp.position);
			const float COLLECT_DISTANCE = 1.0f;
			if (distance < COLLECT_DISTANCE) {
				
				// std::cout << "Collision with " << e.id << std::endl;
				for (std::shared_ptr<EventHandler> handler : eventHandlers) {
					handler.get()->insertPair(egg, e);
				}
				break;
			}


		}

		for (std::shared_ptr<EventHandler> handler : eventHandlers) {
			handler->update();
		}

	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------


	EggMovementSystem::EggMovementSystem(std::shared_ptr<ComponentManager<PositionComponent>> positionCompManager, std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCompManager) {
		positionCM = positionCompManager;
		eggHolderCM = eggHolderCompManager;
	}

	void EggMovementSystem::update() {
		Entity egg = *registeredEntities.begin();
		EggHolderComponent& eggHolder = eggHolderCM->lookup(egg);
		if (eggHolder.holderId >= 0) {
			PositionComponent& eggPos = positionCM->lookup(egg);
			Entity holder = Entity();
			holder.id = eggHolder.holderId;
			PositionComponent& holderPos = positionCM->lookup(holder);
			eggPos.position.x = holderPos.position.x;
			eggPos.position.y = holderPos.position.y + 2;
			eggPos.position.z = holderPos.position.z;
		}
	}

}