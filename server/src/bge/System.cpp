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
			req.forwardDirection = forwardDirection;

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
		// loop through all pairs (no duplicate) and check if there is box collision or not

		const float COLLISION_DISTANCE = 1.0f;


		for (auto it1 = registeredEntities.begin(); it1 != registeredEntities.end(); ++it1) {
			for (auto it2 = std::next(it1); it2 != registeredEntities.end(); ++it2) {
				Entity ent1 = *it1;
				Entity ent2 = *it2;

				PositionComponent& entity1PositionComp = positionCM->lookup(ent1);
				PositionComponent& entity2PositionComp = positionCM->lookup(ent2);

				auto distance = glm::length(entity1PositionComp.position - entity2PositionComp.position);

				if (distance < COLLISION_DISTANCE) {
					for (std::shared_ptr<EventHandler> handler : eventHandlers) {
						handler->insertPair(ent1, ent2);
					}
				}
			}
		}


		for (std::shared_ptr<EventHandler> handler : eventHandlers) {
			handler->update();
		}

	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------


	EggMovementSystem::EggMovementSystem(std::shared_ptr<ComponentManager<PositionComponent>> positionCompManager, 
										 std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCompManager,
										 std::shared_ptr<ComponentManager<MovementRequestComponent>> playerRequestCompManager) {
		positionCM = positionCompManager;
		eggHolderCM = eggHolderCompManager;
		moveReqCM = playerRequestCompManager;
	}

	void EggMovementSystem::update() {
		Entity egg = *registeredEntities.begin();
		EggHolderComponent& eggHolder = eggHolderCM->lookup(egg);
		if (eggHolder.holderId >= 0) {
			PositionComponent& eggPos = positionCM->lookup(egg);
			Entity holder = Entity(eggHolder.holderId);
			PositionComponent& holderPos = positionCM->lookup(holder);
			MovementRequestComponent& req = moveReqCM->lookup(holder);
			eggPos.position = holderPos.position - req.forwardDirection;
			// eggPos.position.x = holderPos.position.x - req.forwardDirection.x;
			// eggPos.position.y = holderPos.position.y;
			// eggPos.position.z = holderPos.position.z - req.forwardDirection.z;
		}
	}

}