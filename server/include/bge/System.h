#pragma once

#include "Entity.h"
#include "ComponentManager.h"
#include "Component.h"
#include "GameConstants.h"
#include "EventHandler.h"


#include <iostream>
#include <set>
#include <bitset>


namespace bge {
	class System {
	public:
		virtual void init();
		virtual void update();

		void registerEntity(Entity entity);
		void deRegisterEntity(Entity entity);
		void addEventHandler(std::shared_ptr<EventHandler> handler);

	protected:
		std::bitset<32> systemSignature;
		std::set<Entity> registeredEntities;

		// each system has a vector of event handler
		// when an event happens, notify all of them
		std::vector<std::shared_ptr<EventHandler>> eventHandlers;

	};

	class MovementSystem : public System {
	public:
		MovementSystem(std::shared_ptr<ComponentManager<PositionComponent>> positionCM, std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM);
		void update();
	protected:
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
		std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
	};

	class PlayerAccelerationSystem : public System {
	public:
		void update();
		PlayerAccelerationSystem(std::shared_ptr<ComponentManager<PositionComponent>> positionCM, std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM, std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM, std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM);
	protected:
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
		std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
		std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM;
		std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM;
	};


	class ProjectileAccelerationSystem : public System {
	public:
		void update();
		ProjectileAccelerationSystem(
			std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
			std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM,
			std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM,
			std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM);
	protected:
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
		std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
	};


	class PlayerVSGroundCollisionSystem : public System {
	public:
		void update();
		PlayerVSGroundCollisionSystem(
			std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
			std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM,
			std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM);
	protected:
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
		std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
		std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM;
	};


	class EggVSPlayerCollisionSystem : public System {
	public:
		void update();
		EggVSPlayerCollisionSystem(
			std::shared_ptr<ComponentManager<PositionComponent>> positionCM, 
			std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM,
			std::shared_ptr<ComponentManager<DimensionComponent>> dimensionCM);

	protected:
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
		std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM;
		std::shared_ptr<ComponentManager<DimensionComponent>> dimensionCM;

		// for the egg vs player system, we will need the eggVsPlayer handler
	};

	class EggMovementSystem: public System {
		public:
			Entity egg; 
			void update();
			EggMovementSystem(
				std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
				std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM);
		protected:
			std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
			std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM;
	};
}

