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
        std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM)
		: positionCM(positionCM), velocityCM(velocityCM) { }

	void PlayerStackingHandler::insertPairAndData(Entity a, Entity b, bool is_top_down_collision) { 
		// std::cout << "PlayerStackingHandler inserts pair " << a.id << " and " << b.id <<  " (is top down collision?: " << is_top_down_collision <<")\n";
		
		if (!is_top_down_collision) {
			return;
		}
		
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

		std::printf("Player %d stands on top of entity %d\n", top.id, bottom.id);

		// reset the player's downward velocity (so it stands) and jump count
		
		


	}

	void PlayerStackingHandler::update() {

	}

}