#include "bge/EventHandler.h"
#include "bge/World.h"

namespace bge {
	EventHandler::EventHandler() {}

	void EventHandler::insertOneEntity(Entity a) {}
	void EventHandler::insertPair(Entity a, Entity b) {}
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
		: positionCM(positionCM), velocityCM(velocityCM) { 
			// std::cout << "Player Stacking Handler created\n";
		}

	void PlayerStackingHandler::insertPair(Entity a, Entity b){

	}

	void PlayerStackingHandler::insertPair(Entity a, Entity b, int ){ 
		//todo pass in additional parameters to handle player stacking?

	}

	void PlayerStackingHandler::update() {

	}

}