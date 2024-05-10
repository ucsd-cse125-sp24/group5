#include "bge/EventHandler.h"
#include "bge/World.h"

namespace bge {
	EventHandler::EventHandler() {}

	void EventHandler::insertOneEntity(Entity a) {}
	void EventHandler::insertPair(Entity a, Entity b) {}
	void EventHandler::insertPairAndData(Entity a, Entity b, bool is_top_down_collision) {
		insertPair(a,b);
	}

	void EventHandler::update() {}

	// void EventHandler::registerEntity(Entity entity) {
	// 	registeredEntities.insert(entity);
	// }
	// void EventHandler::deregisterEntity(Entity entity) {
	// 	auto it = registeredEntities.find(entity);
	// 	if (it != registeredEntities.end()) {
	// 		registeredEntities.erase(it);
	// 	}
	// }
	// bool EventHandler::checkExist(Entity entity) {
	// 	auto it = registeredEntities.find(entity);
	// 	return it != registeredEntities.end();
	// }


	ProjectileVsPlayerHandler::ProjectileVsPlayerHandler(
		std::shared_ptr<ComponentManager<HealthComponent>> healthCM
	) : EventHandler(), healthCM(healthCM) {}

	void ProjectileVsPlayerHandler::insertPair(Entity firstEntity, Entity secondEntity) {

		// first, check that both these entities are in our handler list of interest
		// if (!checkExist(firstEntity) || !checkExist(secondEntity)) return;

		// alan: imo handler != system, so handler shouldn't keep track of a list of registeredEntities like systems do. 
		// handler should just handle event between entities. 

		Entity player;
		Entity projectile;

		if (firstEntity.type == PLAYER && secondEntity.type == PROJECTILE) {
			// firstEntity exist in healthCM, this means this is player entity
			player = firstEntity;
			projectile = secondEntity;
		}
		else if (secondEntity.type == PLAYER && firstEntity.type == PROJECTILE) {
			player = secondEntity;
			projectile = firstEntity;
		}
		else {
			return;
		}

		// then insert this pair into our list of interest

	}

	void ProjectileVsPlayerHandler::update() {
		// update components belong to entities of our interest
	}


	// ------------------------------------------------------------------------------------------------------------------------------------------------


	EggVsPlayerHandler::EggVsPlayerHandler(
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
		std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM
	) : EventHandler(), positionCM(positionCM), eggHolderCM(eggHolderCM), eggChangeOwnerCD(0) {
		time(&timer);
	}


	void EggVsPlayerHandler::insertPair(Entity a, Entity b) {

		// if (!checkExist(a) || !checkExist(b)) return;

		Entity egg;
		Entity player;
		// do a check to ensure we only accept Entity that we should manage
		if (a.type == EGG && b.type == PLAYER) {
			egg = a;
			player = b;
		}
		else if (a.type == PLAYER && b.type == EGG) {
			egg = b;
			player = a;
		}
		else {
			return;
		}

		EggHolderComponent& eggHolderComp = eggHolderCM->lookup(egg);
		if (eggHolderComp.holderId == player.id) {
			return;
		}

		double seconds = difftime(time(nullptr),timer);
		std::printf("Player %d collides with egg (%d) with CD %f\n", player.id, egg.id, seconds);
		if (seconds < EGG_CHANGE_OWNER_CD) {		// wait
			return;
		}
		else {						// assign egg, restart CD
			time(&timer);
		}

		pairsToUpdate.push_back({ egg, player });
	}

	void EggVsPlayerHandler::update() {

		for (const auto& [egg, player] : pairsToUpdate) {
			// update the eggHolderCM pointing to the player
			EggHolderComponent& eggHolderComp = eggHolderCM->lookup(egg);
			eggHolderComp.holderId = player.id;

			// std::cout << "Egg belongs to player " << player.id << std::endl;
		}

		// after the update, we must clear up the list of pairsToUpdate
		pairsToUpdate.clear();
	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------


	PlayerStackingHandler::PlayerStackingHandler(
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
        std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM,
        std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpCM)
		: positionCM(positionCM), velocityCM(velocityCM), jumpCM(jumpCM)  { }

	void PlayerStackingHandler::insertPairAndData(Entity a, Entity b, bool is_top_down_collision) { 
		// std::cout << "PlayerStackingHandler inserts pair " << a.id << " and " << b.id <<  " (is top down collision?: " << is_top_down_collision <<")\n";
		if (is_top_down_collision) {
			handleTopDownCollision(a,b);
		}
		else {
			handleSideToSideCollision(a,b);
		}	
	}

	void PlayerStackingHandler::handleTopDownCollision(Entity a, Entity b) {
		// who's top, who's bottom?
		Entity top;
		Entity bottom;
		PositionComponent& posA = positionCM->lookup(a);
		PositionComponent& posB = positionCM->lookup(b);
		if (posA.position.y > posB.position.y) {
			top = a;
			bottom = b;
		}
		else {
			top = b;
			bottom = a;
		}

		// the top has to be a player, the bottom can be {player, egg, fireball, etc.} (yes you can jump on a moving fireball and rejump if you're skilled enough)
		if (top.type != PLAYER) {
			return;
		}

		// std::printf("Player %d stands on top of entity %d\n", top.id, bottom.id);

		// reset the player's downward velocity / stays on the bottom entity
		// PositionComponent& posTop = positionCM->lookup(top);
		// PositionComponent& posBottom = positionCM->lookup(bottom);
		VelocityComponent& velTop = velocityCM->lookup(top);
		VelocityComponent& velBottom = velocityCM->lookup(bottom);
		velTop.velocity.y = std::max({velBottom.velocity.y, velTop.velocity.y, 0.0f});

		// reset jumps used
		JumpInfoComponent& jumpTop = jumpCM->lookup(top);
		jumpTop.doubleJumpUsed = 0;
	}

	void PlayerStackingHandler::handleSideToSideCollision(Entity a, Entity b) {

		// only handle side-to-side collision among players
		if (a.type != PLAYER || b.type != PLAYER) {
			return;
		}
		// std::printf("Handling side to side collision between entity %d and %d\n", a.id, b.id);

		// Elastic collision: exchange velocities in the xz-plane
		VelocityComponent& velA = velocityCM->lookup(a);
		VelocityComponent& velB = velocityCM->lookup(b);

		// std::printf("Entity A starting velocity is %f, %f\n", velA.velocity.x, velA.velocity.z);
		// std::printf("Entity B starting velocity is %f, %f\n", velB.velocity.x, velB.velocity.z);

		// give them opposite velocitices to separate them apart, based on their collision normal
		PositionComponent& posA = positionCM->lookup(a);
		PositionComponent& posB = positionCM->lookup(b);
		glm::vec3 aToB = glm::normalize(posB.position - posA.position);  
		aToB.y = 0.0f;  // vector was 3D. make it just xz-coordinates
		velA.velocity -= aToB * 0.5f;
		velB.velocity += aToB * 0.5f;

		// Add some extra elasticity just for fun
		velA.velocity.x *= 2;
		velA.velocity.z *= 2;

		velB.velocity.x *= 2;
		velB.velocity.z *= 2;

		//std::printf("Entity A ending velocity is %f, %f\n", velA.velocity.x, velA.velocity.z);
		//std::printf("Entity B ending velocity is %f, %f\n", velB.velocity.x, velB.velocity.z);

		// const float MINI_SPEED = 1;
		// bool playerA_isStaticHorizontally = abs(velA.velocity.x) < MINI_SPEED && abs(velA.velocity.z) < MINI_SPEED;
		// bool playerB_isStaticHorizontally = abs(velB.velocity.x) < MINI_SPEED && abs(velB.velocity.z) < MINI_SPEED;
		// if (playerA_isStaticHorizontally && playerB_isStaticHorizontally) {
			// PositionComponent& posA = positionCM->lookup(a);
			// PositionComponent& posB = positionCM->lookup(b);
			// glm::vec3 aToB = glm::normalize(posB.position - posA.position);  
			// aToB.y = 0.0f;  // vector was 3D. make it just xz-coordinates
			// velA.velocity -= aToB * 0.5f;
			// velB.velocity += aToB * 0.5f;
		// } else {
		// 	std::swap(velA.velocity.x, velB.velocity.x);
		// 	std::swap(velA.velocity.z, velB.velocity.z);

		// 	velA.velocity.x *= 5;
		// 	velA.velocity.z *= 5;

		// 	velB.velocity.x *= 5;
		// 	velB.velocity.z *= 5;
		// }
	}

	void PlayerStackingHandler::update() {

	}

}