#pragma once

#include "Entity.h"
#include "ComponentManager.h"
#include "Component.h"
#include "GameConstants.h"
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

        protected:
            std::bitset<32> systemSignature;
            std::set<Entity> registeredEntities;
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

    class CollisionSystem : public System {
    public:
        void update();
        CollisionSystem(std::shared_ptr<ComponentManager<PositionComponent>> positionCM, std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM, std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM);
    protected:
        std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
        std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
        std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpInfoCM;
    };
    
}

