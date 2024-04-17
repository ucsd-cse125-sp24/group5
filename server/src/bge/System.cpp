#include "bge/System.h"

namespace bge {

    void System::init() {

    }

    void System::update(int dt) {

    }

    void System::registerEntity(Entity entity) {
        registeredEntities.push_back(entity);
    }

    void System::deRegisterEntity(Entity entity) {
        for (auto it = registeredEntities.begin(); it != registeredEntities.end(); ++it) {
            Entity e = *it;
            if (e.id == entity.id) {
                registeredEntities.erase(it);
                return;
            }
        }      
    }

    class MovementSystem : System {
        void update(int dt) {
            
        }
    };
    
}