#include "bge/EventHandler.h"
#include "bge/World.h"

namespace bge {
	EventHandler::EventHandler() {}

	void EventHandler::insertOneEntity(Entity a) {}
	void EventHandler::insertPair(Entity a, Entity b) {}
	void EventHandler::insertPairAndData(Entity a, Entity b, bool is_top_down_collision, 
										float xOverlapDistance, float zOverlapDistance) {
		insertPair(a,b);
	}

	void EventHandler::update() {}

	void EventHandler::registerEntity(Entity entity) {
		registeredEntities.insert(entity);
	}
	void EventHandler::deregisterEntity(Entity entity) {
		auto it = registeredEntities.find(entity);
		if (it != registeredEntities.end()) {
			registeredEntities.erase(it);
		}
	}
	bool EventHandler::checkExist(Entity entity) {
		auto it = registeredEntities.find(entity);
		return it != registeredEntities.end();
	}

	void EventHandler::addWorld(World* parent) {
		world = parent;
	}


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

		if (eggHolderCM->lookup(egg).holderId == player.id) {
			return;
		}

		std::printf("Player %d collides with egg (%d)\n", player.id, egg.id);

		double seconds = difftime(time(nullptr),timer);
		if (seconds < EGG_CHANGE_OWNER_CD) {		// wait
			// std::cout << "Egg CD" << seconds << std::endl;
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

	void PlayerStackingHandler::insertPairAndData(Entity a, Entity b, bool is_top_down_collision,
												  float xOverlapDistance, float zOverlapDistance) { 
		// std::cout << "PlayerStackingHandler inserts pair " << a.id << " and " << b.id <<  " (is top down collision?: " << is_top_down_collision <<")\n";
		if (is_top_down_collision) {
			handleTopDownCollision(a,b);
		}
		else {
			handleSideToSideCollision(a,b, xOverlapDistance, zOverlapDistance);
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
		PositionComponent& posTop = positionCM->lookup(top);
		PositionComponent& posBottom = positionCM->lookup(bottom);
		VelocityComponent& velTop = velocityCM->lookup(top);
		VelocityComponent& velBottom = velocityCM->lookup(bottom);
		posTop.position.y = MAX(posBottom.position.y + PLAYER_Y_HEIGHT, posTop.position.y);
		velTop.velocity.y = MAX3(velBottom.velocity.y, velTop.velocity.y, 0.0f);

		// reset jumps used
		JumpInfoComponent& jumpTop = jumpCM->lookup(top);
		jumpTop.doubleJumpUsed = 0;
	}

	void PlayerStackingHandler::handleSideToSideCollision(Entity a, Entity b, float xOverlapDistance, float zOverlapDistance) {
		std::printf("Handling side to side collision between entity %d and %d\n", a.id, b.id);

		// only handle side-to-side collision among players
		if (a.type != PLAYER || b.type != PLAYER) {
			return;
		}

		// Shit, need to consider the aggregation of x-axis and z-axis!

		// Elastic collision: exchange velocities in the xz-plane
		VelocityComponent& velA = velocityCM->lookup(a);
		VelocityComponent& velB = velocityCM->lookup(b);
		std::swap(velA.velocity.x, velB.velocity.x);
		std::swap(velA.velocity.z, velB.velocity.z);

		// Move the boxes apart (by their overlapping distance) in the xz-plane. 
		const float INERTIA = 1.0f;
		PositionComponent& posA = positionCM->lookup(a);
		PositionComponent& posB = positionCM->lookup(b);
		posA.position.x += velA.velocity.x * INERTIA;
		posA.position.z += velA.velocity.z * INERTIA;
		posB.position.x += velB.velocity.x * INERTIA;
		posB.position.z += velB.velocity.z * INERTIA;

		if (posA.position.x > posB.position.x) {
			posA.position.x += 0.2;
			posB.position.x -= 0.2;
		} else {
			posA.position.x -= 0.2;
			posB.position.x += 0.2;
		}

		if (posA.position.z > posB.position.z) {
			posA.position.z += 0.2;
			posB.position.z -= 0.2;
		} else {
			posA.position.z -= 0.2;
			posB.position.z += 0.2;
		}

		// // move smaller x to smaller, move bigger x to bigger
		// if (posA.position.x < posB.position.x) {
		// 	posA.position.x -= xOverlapDistance / 10.0f + 0.001f; // 0.01 to avoid re-handling side-to-side collision
		// 	posB.position.x += xOverlapDistance / 10.0f + 0.001f;	
		// } else {
		// 	posA.position.x += xOverlapDistance / 10.0f + 0.001f;
		// 	posB.position.x -= xOverlapDistance / 10.0f + 0.001f;	
		// }
		// // move smaller z to smaller, move bigger z to bigger
		// if (posA.position.z < posB.position.z) {
		// 	posA.position.z -= zOverlapDistance / 10.0f + 0.001f;
		// 	posB.position.z += zOverlapDistance / 10.0f + 0.001f;	
		// } else {
		// 	posA.position.z += zOverlapDistance / 10.0f + 0.001f;
		// 	posB.position.z -= zOverlapDistance / 10.0f + 0.001f;	
		// }
		
		
		

	}

	void PlayerStackingHandler::update() {

	}

}