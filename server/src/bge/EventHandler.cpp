#include "bge/EventHandler.h"

namespace bge {
    EventHandler::EventHandler(void(*deleteEntity)(Entity)) {
        deleteEntity_ = deleteEntity;
    }
    void EventHandler::insertOneEntity(Entity a){}
    void EventHandler::insertPair(Entity a, Entity b){}
    void EventHandler::update(){}

    void EventHandler::registerEntity(Entity entity) {
        registeredEntities.insert(entity);
    }
    void EventHandler::deregisterEntity(Entity entity) {
        auto it = registeredEntities.find(entity);
        if (it != registeredEntities.end()) {
            registeredEntities.erase(it);
        }
    }
    bool EventHandler::checkExist(Entity entity) {
        auto it = registeredEntities.find(entity);
        return it != registeredEntities.end();
    }




    ProjectileVSPlayerHandler::ProjectileVSPlayerHandler(
        void(*deleteEntity)(Entity),
        std::shared_ptr<ComponentManager<HealthComponent>> healthCM
    ): EventHandler(deleteEntity), healthCM(healthCM){}

    void ProjectileVSPlayerHandler::insertPair(Entity firstEntity, Entity secondEntity) {

        // first, check that both these entities are in our handler list of interest
        if (!checkExist(firstEntity) || !checkExist(secondEntity)) return;

        Entity player;
        Entity projectile;
        
        if (healthCM.get()->checkExist(firstEntity)) {
            // firstEntity exist in healthCM, this means this is player entity
            player = firstEntity;
            projectile = secondEntity;
        } else {
            player = secondEntity;
            projectile = firstEntity;
        }

        // then insert this pair into our list of interest

    }

    void ProjectileVSPlayerHandler::update() {
        // update components belong to entities of our interest
    }





}