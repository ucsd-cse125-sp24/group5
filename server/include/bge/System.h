#pragma once

#include "Entity.h"
#include "ComponentManager.h"
#include "Component.h"
#include "GameConstants.h"
#include "EventHandler.h"

#include "World.h"
#include <iostream>
#include <set>
#include <bitset>
#include <utility>
#include <random>

namespace bge {

    class World;

	class System {
	public:
		virtual void init();
		virtual void update();

		void registerEntity(Entity entity);
		void deRegisterEntity(Entity entity);
		void addEventHandler(std::shared_ptr<EventHandler> handler);
		size_t size();

	protected:
		World* world;

		std::bitset<32> systemSignature;
		std::set<Entity> registeredEntities;

		// each system has a vector of event handler
		// when an event happens, notify all of them
		std::vector<std::shared_ptr<EventHandler>> eventHandlers;

	};

	class BoxCollisionSystem : public System {
	public:
		void update();
		BoxCollisionSystem(
			World* gameWorld,
			std::shared_ptr<ComponentManager<PositionComponent>> positionCM, 
			std::shared_ptr<ComponentManager<EggInfoComponent>> eggInfoCM,
			std::shared_ptr<ComponentManager<BoxDimensionComponent>> boxDimensionCM);

	protected:
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
		std::shared_ptr<ComponentManager<EggInfoComponent>> eggInfoCM;
		std::shared_ptr<ComponentManager<BoxDimensionComponent>> boxDimensionCM;

		// for the egg vs player system, we will need the eggVsPlayer handler
	};

	class EggMovementSystem: public System {
		public:
			void update();
			EggMovementSystem(
				World* gameWorld,
				std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
				std::shared_ptr<ComponentManager<EggInfoComponent>> eggInfoCM,
				std::shared_ptr<ComponentManager<MovementRequestComponent>> playerRequestCompManager,
				std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataCM);
		protected:
			std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
			std::shared_ptr<ComponentManager<EggInfoComponent>> eggInfoCM;
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
            PlayerAccelerationSystem(World* gameWorld, std::shared_ptr<ComponentManager<PositionComponent>> positionCM, std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM, std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM, std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM, std::shared_ptr<ComponentManager<StatusEffectsComponent>> statusEffectsCM);
        protected:
            std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
            std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
            std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM;
            std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM;
			std::shared_ptr<ComponentManager<StatusEffectsComponent>> statusEffectsCM;
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

	class BulletSystem : public System {
		public:
			BulletSystem(World* _world, std::shared_ptr<ComponentManager<PositionComponent>> _positionCM, std::shared_ptr<ComponentManager<MovementRequestComponent>> _movementRequestCM, std::shared_ptr<ComponentManager<CameraComponent>> _cameraCM,
			std::shared_ptr<ComponentManager<PlayerDataComponent>> _playerDataCM, std::shared_ptr<ComponentManager<HealthComponent>> healthCM, std::shared_ptr<ComponentManager<StatusEffectsComponent>> _statusCM);
			void update();
		protected:
			std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
			std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM;
			std::shared_ptr<ComponentManager<CameraComponent>> cameraCM;
			std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataCM;
			std::shared_ptr<ComponentManager<StatusEffectsComponent>> statusCM;
	};

	class SeasonAbilitySystem : public System {
	public:
		void update();
		SeasonAbilitySystem(
			World* gameWorld,
			std::shared_ptr<ComponentManager<MovementRequestComponent>> playerRequestComponentManager,
			std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataComponentManager,
			std::shared_ptr<ComponentManager<SeasonAbilityStatusComponent>> seasonAbilityStatusComponentManager,
			std::shared_ptr<ComponentManager<BallProjDataComponent>> ballProjDataComponentManager,
			std::shared_ptr<ComponentManager<PositionComponent>> positionComponentManager,
			std::shared_ptr<ComponentManager<VelocityComponent>> velocityComponentManager,
			std::shared_ptr<ComponentManager<CameraComponent>> cameraComponentManager);
	protected:
		std::shared_ptr<ComponentManager<MovementRequestComponent>> moveReqCM;
		std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataCM;
		std::shared_ptr<ComponentManager<SeasonAbilityStatusComponent>> seasonAbilityStatusCM;
		std::shared_ptr<ComponentManager<BallProjDataComponent>> ballProjDataCM;
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
		std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
		std::shared_ptr<ComponentManager<CameraComponent>> cameraCM;
	};

	class ProjectileStateSystem : public System {
	public:
		void update();
		ProjectileStateSystem(
			World* gameWorld,
			std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataComponentManager,
			std::shared_ptr<ComponentManager<StatusEffectsComponent>> statusEffectsComponentManager,
			std::shared_ptr<ComponentManager<BallProjDataComponent>> ballProjDataComponentManager,
			std::shared_ptr<ComponentManager<PositionComponent>> positionComponentManager,
			std::shared_ptr<ComponentManager<VelocityComponent>> velocityComponentManager,
			std::shared_ptr<ComponentManager<MeshCollisionComponent>> meshCollisionComponentManager,
			std::shared_ptr<ComponentManager<HealthComponent>> healthComponentManagaer);
	protected:
		std::shared_ptr<ComponentManager<PlayerDataComponent>> playerDataCM;
		std::shared_ptr<ComponentManager<StatusEffectsComponent>> statusEffectsCM;
		std::shared_ptr<ComponentManager<BallProjDataComponent>> ballProjDataCM;
		std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
		std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
		std::shared_ptr<ComponentManager<MeshCollisionComponent>> meshCollisionCM;
		std::shared_ptr<ComponentManager<HealthComponent>> healthCM;
	};

	class SeasonEffectSystem : public System {
	public:
		void update();
		SeasonEffectSystem(
			World* gameWorld,
			std::shared_ptr<ComponentManager<HealthComponent>> healthCM,
			std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM,
        	std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM, 
        	std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM,
			std::shared_ptr<ComponentManager<SeasonAbilityStatusComponent>> seasonAbilityStatusCM
		);
	protected:
		std::shared_ptr<ComponentManager<HealthComponent>> healthCM;
		std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
        std::shared_ptr<ComponentManager<MovementRequestComponent>> movementRequestCM;
        std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM;
		std::shared_ptr<ComponentManager<SeasonAbilityStatusComponent>> seasonAbilityStatusCM;
	};

	class LerpingSystem : public System {
	public:
		void update();
		LerpingSystem(World* _world);
	};

	class DanceBombSystem : public System {
	public:
		void update();
		DanceBombSystem(World* _world);
	protected:
		time_t danceBombTimes[DANCE_BOMBS_PER_GAME];
		unsigned int nextDanceBomb = 0;
	};

	class GodMovementSystem : public System {
	public:
		void update();
		GodMovementSystem(World* _world);
	private:
		glm::vec3 position; 
	};
}