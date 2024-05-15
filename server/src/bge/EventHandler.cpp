#include "bge/EventHandler.h"
#include "bge/World.h"

namespace bge {
	EventHandler::EventHandler() {}

	void EventHandler::handleInteraction(Entity a, Entity b) {}
	void EventHandler::handleInteractionWithData(Entity a, Entity b, bool, float) {
		handleInteraction(a,b);
	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------


	BulletVsPlayerHandler::BulletVsPlayerHandler(
		std::shared_ptr<ComponentManager<HealthComponent>> healthCM,
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM
	) : EventHandler(), healthCM(healthCM), positionCM(positionCM) {}

	void BulletVsPlayerHandler::handleInteraction(Entity shooter, Entity target) {

		HealthComponent& targetHealth = healthCM->lookup(target);
		targetHealth.healthPoint -= 10;

		std::printf("player %d has %d hp left\n", target.id, targetHealth.healthPoint);
		
		// Maybe switch positions in the future?
		if (targetHealth.healthPoint <= 0) {
			PositionComponent& posA = positionCM->lookup(shooter);
			PositionComponent& posB = positionCM->lookup(target);
			glm::vec3 temp = posA.position;
			posA.position = posB.position;
			posB.position = temp;
		}

	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------


	EggVsPlayerHandler::EggVsPlayerHandler(
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
		std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM
	) : EventHandler(), positionCM(positionCM), eggHolderCM(eggHolderCM), eggChangeOwnerCD(0) {
		time(&timer);
	}


	void EggVsPlayerHandler::handleInteraction(Entity a, Entity b) {

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
		// std::printf("Player %d collides with egg (%d) with CD %f\n", player.id, egg.id, seconds);
		if (eggHolderComp.isThrown && eggHolderComp.throwerId != player.id) {
			eggHolderComp.isThrown = false;
			time(&timer);
		}
		else if (seconds < EGG_CHANGE_OWNER_CD) {		// wait
			return;
		}
		else {						// assign egg, restart CD
			time(&timer);
		}

		// pairsToUpdate.push_back({ egg, player });
		eggHolderComp.holderId = player.id;
	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------


	PlayerStackingHandler::PlayerStackingHandler(
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
        std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM,
        std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpCM)
		: positionCM(positionCM), velocityCM(velocityCM), jumpCM(jumpCM)  { }

	void PlayerStackingHandler::handleInteractionWithData(Entity a, Entity b, bool is_top_down_collision, float yOverlapDistance) { 
		// std::cout << "PlayerStackingHandler inserts pair " << a.id << " and " << b.id <<  " (is top down collision?: " << is_top_down_collision <<")\n";
		if (is_top_down_collision) {
			handleTopDownCollision(a,b, yOverlapDistance);
		}
		else {
			handleSideToSideCollision(a,b);
		}	
	}

	void PlayerStackingHandler::handleTopDownCollision(Entity a, Entity b, float yOverlapDistance) {
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

		// the top has to be a player, the bottom can be {player, fireball, etc.} (yes you can jump on a moving fireball and rejump if you're skilled enough)
		if (top.type != PLAYER || bottom.type == EGG) {
			return;
		}

		// std::printf("Player %d stands on top of entity %d\n", top.id, bottom.id);

		// reset the player's downward velocity / stays on the bottom entity
		// PositionComponent& posTop = positionCM->lookup(top);
		// PositionComponent& posBottom = positionCM->lookup(bottom);
		VelocityComponent& velTop = velocityCM->lookup(top);
		VelocityComponent& velBottom = velocityCM->lookup(bottom);
		velTop.velocity.y = std::max({velBottom.velocity.y, velTop.velocity.y, std::abs(yOverlapDistance)});

		// fix: prevent box clipping by moving the top player up by at least their overlap distance ^

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

}