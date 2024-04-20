#include "bge/System.h"

namespace bge {

    void System::init() {

    }

    void System::update(int dt) {

    }

    void System::registerEntity(Entity entity) {
        registeredEntities.insert(entity);
    }

    void System::deRegisterEntity(Entity entity) {
        auto it = registeredEntities.find(entity);
        if (it != registeredEntities.end()) {
            registeredEntities.erase(it);
        }
    }
    
}