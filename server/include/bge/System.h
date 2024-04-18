#pragma once

#include "Entity.h"
#include "ComponentManager.h"
#include "Component.h"
#include <vector>
#include <bitset>

namespace bge {
    class System {
        public:
            void init();
            void update(int dt);
        
            void registerEntity(Entity entity);
            void deRegisterEntity(Entity entity);

        protected:
            std::bitset<32> systemSignature;
            std::vector<Entity> registeredEntities;
    };
    
}

