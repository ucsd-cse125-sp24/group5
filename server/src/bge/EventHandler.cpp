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

		if (healthCM.get()->checkExist(firstEntity)) {
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


	EggVsPlayerHandler::EggVsPlayerHandler(
		void(*deleteEntity)(Entity),
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
		std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM
	) : EventHandler(deleteEntity), positionCM(positionCM), eggHolderCM(eggHolderCM) {}


	void EggVsPlayerHandler::insertPair(Entity a, Entity b) {
		Entity egg;
		Entity player;

		if (eggHolderCM.get()->checkExist(a)) {
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
			EggHolderComponent& eggHolderComp = eggHolderCM.get()->lookup(egg);
			eggHolderComp.holderId = player.id;

			std::cout << "Update to " << player.id << std::endl;

			// update the egg position to be close to the player
			PositionComponent& eggPositionComp = positionCM.get()->lookup(egg);
			PositionComponent& playerPositionComp = positionCM.get()->lookup(player);
			// for now just hardcoded to be 2 away from player
			eggPositionComp.position.x = playerPositionComp.position.x + 1;
			eggPositionComp.position.y = playerPositionComp.position.y;
			eggPositionComp.position.z = playerPositionComp.position.z;
		}

		// after the update, we must clear up the list of pairsToUpdate
		pairsToUpdate.clear();
	}





}