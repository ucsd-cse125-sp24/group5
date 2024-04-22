#include "bge/System.h"

namespace bge {

    void System::init() {
    }

    void System::update() {
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

    void MovementSystem::update() {
        std::cout << "movment system updating\n";

    }

    void PlayerAccelerationSystem::update() {
        std::cout << "player acceleration system updating\n";
    }
    
}