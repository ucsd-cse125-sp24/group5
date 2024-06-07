#include "bge/EventHandler.h"
#include "bge/World.h"

namespace bge {
	EventHandler::EventHandler() {}

	void EventHandler::handleInteraction(Entity a, Entity b) {}
	void EventHandler::handleInteractionWithData(Entity a, Entity b, bool, float) {
		handleInteraction(a,b);
	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------


	ProjectileVsPlayerHandler::ProjectileVsPlayerHandler(
		std::shared_ptr<ComponentManager<BallProjDataComponent>> ballProjDataCM
	) : EventHandler(), projDataCM(ballProjDataCM) {}

	void ProjectileVsPlayerHandler::handleInteraction(Entity firstEntity, Entity secondEntity) {
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

		
		BallProjDataComponent& projData = projDataCM->lookup(projectile);
		if (projData.creatorId != player.id) {
			projData.collidedWithPlayer = true;
			projData.collisionPlayerId = player.id;
		}
	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------

	BulletVsPlayerHandler::BulletVsPlayerHandler(
		World* _world,
		std::shared_ptr<ComponentManager<HealthComponent>> healthCM,
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM
	) : EventHandler(), healthCM(healthCM), positionCM(positionCM) {
		eggInfoCM = _world->eggInfoCM;
		world = _world;
	}

	void BulletVsPlayerHandler::handleInteraction(Entity shooter, Entity target) {
		if (target.type != PLAYER) {
			return;
		}

		HealthComponent& targetHealth = healthCM->lookup(target);
		targetHealth.healthPoint -= 10;

		// std::printf("player %d has %d hp left\n", target.id, targetHealth.healthPoint);
		
		// switch positions if target is 'dead'
		if (targetHealth.healthPoint <= 0) {
			targetHealth.healthPoint = PLAYER_MAX_HEALTH;

			PositionComponent& posA = positionCM->lookup(shooter);
			PositionComponent& posB = positionCM->lookup(target);
			// glm::vec3 temp = posA.position;
			// posA.position = posB.position;
			// posB.position = temp;
			// std::swap(posA.position, posB.position);
			// todo: maybe linearly interpolate this position exchange in a few frames^ ? 
			world->addComponent(shooter, LerpingComponent(posA.position, posB.position));
			world->addComponent(target, LerpingComponent(posB.position, posA.position));
			posA.isLerping = true;
			posB.isLerping = true;
			world->velocityCM->lookup(shooter).velocity = glm::vec3(0);
			world->velocityCM->lookup(target).velocity = glm::vec3(0);

			Entity egg = world->getEgg();
			EggInfoComponent& eggInfoComp = eggInfoCM->lookup(egg);
			if (eggInfoComp.holderId == target.id) {
				// Allows shooter to pick up egg instantly...basically act like it was thrown
                eggInfoComp.throwerId = target.id;
                eggInfoComp.holderId = shooter.id; 
                eggInfoComp.isThrown = false;
			}
			else if (eggInfoComp.holderId == shooter.id) {
				// egg follows the successful shooter
				PositionComponent& posEgg = positionCM->lookup(egg);
				// posEgg = posA.position;
				glm::vec3 eggDelta = posB.position - posA.position;
				world->addComponent(egg, LerpingComponent(posEgg.position, posEgg.position + eggDelta));
				posEgg.isLerping = true;
				world->velocityCM->lookup(egg).velocity = glm::vec3(0);
			}
		}

	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------


	EggVsPlayerHandler::EggVsPlayerHandler(
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
		std::shared_ptr<ComponentManager<EggInfoComponent>> eggInfoCM
	) : EventHandler(), positionCM(positionCM), eggInfoCM(eggInfoCM), eggChangeOwnerCD(0) {

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


		EggInfoComponent& eggInfoComp = eggInfoCM->lookup(egg);
		if (eggInfoComp.holderId == player.id) {
			return;
		}

		if (eggInfoComp.eggIsDancebomb) {
			handleDancebombVsPlayer(eggInfoComp, player);
			return;
		}

		double seconds = difftime(time(nullptr),eggInfoComp.throwTimer);
		// std::printf("Player %d collides with egg (%d) with CD %f\n", player.id, egg.id, seconds);
		if (eggInfoComp.isThrown && eggInfoComp.throwerId != player.id) {
			eggInfoComp.isThrown = false;
			time(&eggInfoComp.throwTimer);
		}
		else if (seconds < EGG_CHANGE_OWNER_CD) {		// wait
			return;
		}
		else {						// assign egg, restart CD
			time(&eggInfoComp.throwTimer);
		}

		// pairsToUpdate.push_back({ egg, player });
		eggInfoComp.holderId = player.id;
	}

	void EggVsPlayerHandler::handleDancebombVsPlayer(EggInfoComponent& bombInfo, Entity player) {

		// case1: player collides with the lonely dancebomb (that hasn't been thrown by anyone)
		if (bombInfo.holderId < 0 && !bombInfo.bombIsThrown) {
			std::printf("player %d picks up the dancebomb\n", player.id);
			bombInfo.holderId = player.id;
			bombInfo.detonationTicks = DANCE_BOMB_DENOTATION_TICKS_HOLD;
		}

		// else, the dancebomb is already picked up (colliding with owner), ignore.

		// else if the dancebomb is thrown and touches ppl (other than the thrower), start detonation. 
		// (normally, dancebomb detonates based on its timer; but if hits player, then shorten its detonation time to 3 ticks or less
		if (bombInfo.bombIsThrown && bombInfo.throwerId != player.id && !bombInfo.danceInAction) {
			// std::printf("thrown dancebomb hits player %d\n", player.id);
			bombInfo.detonationTicks = std::min(5, bombInfo.detonationTicks);
		}

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
		if (top.type != PLAYER || bottom.type != PLAYER) {
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
		
		// make sure top player displays idle animation if not moving
		if (glm::length(velTop.velocity) < 0.01f) {
			velTop.velocity = glm::vec3(0); 
		}

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