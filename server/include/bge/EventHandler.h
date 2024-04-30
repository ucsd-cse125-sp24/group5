#pragma once

#include <unordered_set>

#include "Entity.h"
#include "Component.h"
#include "ComponentManager.h"
#include "GameConstants.h"

namespace bge {

    class World;

    class EventHandler {
    public:
        EventHandler();

        virtual void insertOneEntity(Entity a);
        virtual void insertPair(Entity a, Entity b);

        // this will run through the list of interest and do appropriate update
        virtual void update();

        void registerEntity(Entity entity);
        void deregisterEntity(Entity entity);
        // check if the entity is in our handler's list of interest
        bool checkExist(Entity entity);

        void addWorld(World* world);

    protected:
        // list of entities this event handler interest on
        std::unordered_set<Entity, Entity::HashFunction> registeredEntities;

        // list of entities that we will perform update on
        // for now, an event handler shall use one or the other, and not both as target of update
        std::vector<Entity> entitiesToUpdate;
        std::vector<std::pair<Entity, Entity>> pairsToUpdate;

        // pointers to function to entity manipulation functions
        void (World::*deleteEntity_)(Entity);
        World* world;
    };

    class ProjectileVsPlayerHandler : public EventHandler {
    public:
        ProjectileVsPlayerHandler(
            std::shared_ptr<ComponentManager<HealthComponent>> healthCM
        );

        void insertPair(Entity a, Entity b);
        void update();

        std::shared_ptr<ComponentManager<HealthComponent>> healthCM;
    };


    class EggVsPlayerHandler : public EventHandler {
    public:
        EggVsPlayerHandler(
            std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
            std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM
        );

        void insertPair(Entity a, Entity b);
        void update();

        std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
        std::shared_ptr<ComponentManager<EggHolderComponent>> eggHolderCM;

    private:
        int eggChangeOwnerCD = 0;
    };

}