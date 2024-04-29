#include "bge/EventHandler.h"

namespace bge {
	EventHandler::EventHandler(void(*deleteEntity)(Entity)) {
		deleteEntity_ = deleteEntity;
	}
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




	ProjectileVsPlayerHandler::ProjectileVsPlayerHandler(
		void(*deleteEntity)(Entity),
		std::shared_ptr<ComponentManager<HealthComponent>> healthCM
	) : EventHandler(deleteEntity), healthCM(healthCM) {}

	void ProjectileVsPlayerHandler::insertPair(Entity firstEntity, Entity secondEntity) {

		// first, check that both these entities are in our handler list of interest
		if (!checkExist(firstEntity) || !checkExist(secondEntity)) return;

		Entity player;
		Entity projectile;

		if (healthCM->checkExist(firstEntity)) {
			// firstEntity exist in healthCM, this means this is player entity
			player = firstEntity;
			projectile = secondEntity;
		}
		else {
			player = secondEntity;
			projectile = firstEntity;
		}

		// then insert this pair into our list of interest

	}

	void ProjectileVsPlayerHandler::update() {
		// update components belong to entities of our interest
	}


	// ------------------------------------------------------------------------------------------------------------------------------------------------


	EggVsPlayerHandler::EggVsPlayerHandler(
		void(*deleteEntity)(Entity),
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
		std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM
	) : EventHandler(deleteEntity), positionCM(positionCM), eggHolderCM(eggHolderCM), eggChangeOwnerCD(0) {}


	void EggVsPlayerHandler::insertPair(Entity a, Entity b) {

		if (eggChangeOwnerCD > 0) {		// wait
			eggChangeOwnerCD--;
			return;
		} else {						// assign egg, restart CD
			eggChangeOwnerCD = EGG_CHANGE_OWNER_CD;
		}

		Entity egg;
		Entity player;

		if (eggHolderCM->checkExist(a)) {
			egg = a;
			player = b;
		}
		else {
			egg = b;
			player = a;
		}

		pairsToUpdate.push_back({ egg, player });
	}

	void EggVsPlayerHandler::update() {
		
		for (const auto& [egg, player] : pairsToUpdate) {
			// update the eggHolderCM pointing to the player
			EggHolderComponent& eggHolderComp = eggHolderCM->lookup(egg);
			eggHolderComp.holderId = player.id;

			std::cout << "Egg belongs to player " << player.id << std::endl;
		}

		// after the update, we must clear up the list of pairsToUpdate
		pairsToUpdate.clear();
	}





}