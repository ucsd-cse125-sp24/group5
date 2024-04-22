#pragma once

#include "Entity.h"
#include "ComponentManager.h"
#include "Component.h"
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
        void update();
    };

    class PlayerAccelerationSystem : public System {
        void update();
    };
    
}

