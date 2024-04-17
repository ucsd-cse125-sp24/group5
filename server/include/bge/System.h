#pragma once

#include "Entity.h"
#include <vector>
#include <bitset>

namespace bge {
    class System {
        public:
            void init();
            void update(int dt);
        
            void registerEntity(Entity entity);
            void deRegisterEntity(Entity entity);

        private:
            std::bitset<32> systemSignature;
            std::vector<Entity> registeredEntities;
    };
    
}

