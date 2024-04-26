#pragma once

#include <unordered_set>
#include <memory>

#include "Entity.h"
#include "Component.h"
#include "ComponentManager.h"

namespace bge {
    class EventHandler {
    public:
        EventHandler(
            void(*deleteEntity)(Entity)
        );

        virtual void update(Entity a);
        virtual void update(Entity a, Entity b);

        void registerEntity(Entity entity);
        void deregisterEntity(Entity entity);
        // check if the entity is in our handler's list of interest
        bool checkExist(Entity entity);

    protected:
        // list of entities this event handler interest on
        std::unordered_set<Entity, Entity::HashFunction> registeredEntities;
        // pointers to function to entity manipulation functions
        void (*deleteEntity_)(Entity);
    };




    class ProjectileVSPlayerHandler : public EventHandler {
    public:
        ProjectileVSPlayerHandler(
            void(*deleteEntity)(Entity),
            std::shared_ptr<ComponentManager<HealthComponent>> healthCM
        );

        void update(Entity a, Entity b);

        std::shared_ptr<ComponentManager<HealthComponent>> healthCM;
    };

}