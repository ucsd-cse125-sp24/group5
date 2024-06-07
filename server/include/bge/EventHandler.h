#pragma once

#include <unordered_set>
#include <time.h> 

#include "Entity.h"
#include "Component.h"
#include "ComponentManager.h"
#include "GameConstants.h"

namespace bge {

    class World;

    class EventHandler {
    public:
        EventHandler();
        virtual void handleInteraction(Entity a, Entity b);
        virtual void handleInteractionWithData(Entity a, Entity b, bool, float);

    protected:
        World* world;
    };

    class ProjectileVsPlayerHandler : public EventHandler {
    public:
        ProjectileVsPlayerHandler(
            std::shared_ptr<ComponentManager<BallProjDataComponent>> projDataCM
        );

        void handleInteraction(Entity a, Entity b);

        std::shared_ptr<ComponentManager<BallProjDataComponent>> projDataCM;
    };

    class BulletVsPlayerHandler : public EventHandler {
    public:
        BulletVsPlayerHandler(
            World* world,
            std::shared_ptr<ComponentManager<HealthComponent>> healthCM,
            std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
            std::shared_ptr<ComponentManager<StatusEffectsComponent>> statusCM
        );

        void handleInteraction(Entity a, Entity b);

        std::shared_ptr<ComponentManager<HealthComponent>> healthCM;
        std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
        std::shared_ptr<ComponentManager<StatusEffectsComponent>> statusCM;
        std::shared_ptr<ComponentManager<EggInfoComponent>> eggInfoCM;
    };

    class EggVsPlayerHandler : public EventHandler {
    public:
        EggVsPlayerHandler(
            std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
            std::shared_ptr<ComponentManager<EggInfoComponent>> eggInfoCM
        );

        void handleInteraction(Entity a, Entity b);

        std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
        std::shared_ptr<ComponentManager<EggInfoComponent>> eggInfoCM;

    private:
        void handleDancebombVsPlayer(EggInfoComponent& bombInfo, Entity player);

        int eggChangeOwnerCD = 0;
        time_t timer;
    };

    class PlayerStackingHandler : public EventHandler {
    public:
        PlayerStackingHandler(
            std::shared_ptr<ComponentManager<PositionComponent>> positionCM,
            std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM,
            std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpCM
        );

        void handleInteractionWithData(Entity a, Entity b, bool is_top_down_collision, float yOverlapDistance);
        void handleTopDownCollision(Entity a, Entity b, float yOverlapDistance);
        void handleSideToSideCollision(Entity a, Entity b);

        std::shared_ptr<ComponentManager<PositionComponent>> positionCM;
        std::shared_ptr<ComponentManager<VelocityComponent>> velocityCM;
        std::shared_ptr<ComponentManager<JumpInfoComponent>> jumpCM;
    };

}