#pragma once
#include <World.h>
#include <Entity.h>
#include <vector>
#include <bitset>

namespace bge {
    class System {
        public:
            void init() {}
            void update(int dt) {}
        
            void registerWorld(World * world) {}
            void registerEntity(Entity entity) {}
            void deRegisterEntity(Entity entity) {}

        private:
            std::bitset<32> systemSignature;
            std::vector<Entity> registeredEntities;
            World * parentWorld;
    };
    
}

