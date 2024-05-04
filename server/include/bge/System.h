#pragma once

#include "Entity.h"
#include "ComponentManager.h"
#include "Component.h"
#include "GameConstants.h"
#include "EventHandler.h"
#include "ShittyUtils.h"

#include "World.h"
#include <iostream>
#include <set>
#include <bitset>
#include <utility>

namespace bge {

    class World;

	class System {
	public:
		virtual void init();
		virtual void update();

		void registerEntity(Entity entity);
		void deRegisterEntity(Entity entity);
		void addEventHandler(std::shared_ptr<EventHandler> handler);

	protected:
		World* world;

		std::bitset<32> systemSignature;
		std::set<Entity> registeredEntities;

		// each system has a vector of event handler
		// when an event happens, notify all of them
		std::vector<std::shared_ptr<EventHandler>> eventHandlers;

	};

	class ProjectileAccelerationSystem : public System {
	public:
		void update();
		ProjectileAccelerationSystem(
			World* gameWorld,
			std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
			std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM,
			std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM,
			std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM);
	protected:
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
		std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
	};

	class BoxCollisionSystem : public System {
	public:
		void update();
		BoxCollisionSystem(
			World* gameWorld,
			std::shared_ptr<ComponentManager<PositionComponent>> positionCM, 
			std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM,
			std::shared_ptr<ComponentManager<BoxDimensionComponent>> dimensionCM);

	protected:
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
		std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM;
		std::shared_ptr<ComponentManager<BoxDimensionComponent>> dimensionCM;

		// for the egg vs player system, we will need the eggVsPlayer handler
	};

	class EggMovementSystem: public System {
		public:
			void update();
			EggMovementSystem(
				World* gameWorld,
				std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
				std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM,
				std::shared_ptr<ComponentManager<MovementRequestComponent>> playerRequestCompManager,
				std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataCM);
		protected:
			std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
			std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM;
			std::shared_ptr<ComponentManager<MovementRequestComponent>> moveReqCM;
			std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataCM;
	};

    class MovementSystem : public System {
        public:
            MovementSystem(World* gameWorld, std::shared_ptr<ComponentManager<PositionComponent>> positionCM, std::shared_ptr<ComponentManager<MeshCollisionComponent>> meshCollisionComponentManager, std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM);
            void update();
        protected:
            std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
            std::shared_ptr<ComponentManager<MeshCollisionComponent>> meshCollisionCM;
            std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
    };

    class PlayerAccelerationSystem : public System {
        public:
            void update();
            PlayerAccelerationSystem(World* gameWorld, std::shared_ptr<ComponentManager<PositionComponent>> positionCM, std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM, std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM, std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM);
        protected:
            std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
            std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
            std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM;
            std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM;
    };

    class CameraSystem : public System {
        public:
            CameraSystem(World* _world, std::shared_ptr<ComponentManager<PositionComponent>> _positionCM, std::shared_ptr<ComponentManager<MovementRequestComponent>> _movementRequestCM, std::shared_ptr<ComponentManager<CameraComponent>> _cameraCM);
            void update();
        protected:
            std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
            std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM;
            std::shared_ptr<ComponentManager<CameraComponent>> cameraCM;
    };

    class CollisionSystem : public System {
    public:
        void update();
        CollisionSystem(World* gameWorld, std::shared_ptr<ComponentManager<PositionComponent>> positionCM, std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM, std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM);
    protected:
        std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
        std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
        std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM;
    };
    
}

