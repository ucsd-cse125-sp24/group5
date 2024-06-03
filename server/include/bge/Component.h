#pragma once

#include <glm/glm.hpp>
#include "GameConstants.h"
#include <vector>
#include <iostream>
#include <time.h> 

namespace bge {

    template <typename ComponentType>
    struct Component {

    };

    struct PositionComponent: Component<PositionComponent> {
        PositionComponent(float x, float y, float z) {
            position = glm::vec3(x,y,z);
        }
        PositionComponent(glm::vec3 pos) {
            position = pos;
        }
        glm::vec3 position;
        bool isLerping = false;
    };

    struct VelocityComponent : Component<VelocityComponent> {
        VelocityComponent(float vx, float vy, float vz) {
            velocity = glm::vec3(vx, vy, vz);
            timeOnGround = 0;
        }
        glm::vec3 velocity;
        int timeOnGround;
        bool onGround = false; // uhhhhhhhhhhhhhhhhhh --alan
    };

    struct JumpInfoComponent : Component<JumpInfoComponent> {
        JumpInfoComponent(int doubleJumpUsed, bool jumpHeld) : doubleJumpUsed(doubleJumpUsed), jumpHeld(jumpHeld) {}
        int doubleJumpUsed;
        bool jumpHeld;
    };

    struct MovementRequestComponent : Component<MovementRequestComponent> {
        MovementRequestComponent(bool forwardRequested, bool backwardRequested, bool leftRequested, bool rightRequested, bool jumpRequested, bool throwEggRequested, bool shootRequested, bool abilityRequested, bool resetRequested, float pitch, float yaw)
            : forwardRequested(forwardRequested), backwardRequested(backwardRequested), leftRequested(leftRequested), rightRequested(rightRequested), 
                jumpRequested(jumpRequested), throwEggRequested(throwEggRequested), shootRequested(shootRequested), abilityRequested(abilityRequested), resetRequested(resetRequested), pitch(pitch), yaw(yaw) {

        }
        bool forwardRequested, backwardRequested, leftRequested, rightRequested, jumpRequested, throwEggRequested, shootRequested, abilityRequested, resetRequested;
        float yaw, pitch;
        glm::vec3 forwardDirection;
        glm::vec3 rightwardDirection;
    };

    struct HealthComponent : Component<HealthComponent> {
        HealthComponent(int healthPoint) : healthPoint(healthPoint) {
        }
        int healthPoint;
    };

    struct StatusEffectsComponent : Component<StatusEffectsComponent> {
        StatusEffectsComponent(float alternateMovementSpeed, unsigned int movementSpeedTicksLeft, unsigned int swappedControlsTicksLeft) : alternateMovementSpeed(alternateMovementSpeed), movementSpeedTicksLeft(movementSpeedTicksLeft), swappedControlsTicksLeft(swappedControlsTicksLeft) {}
        float alternateMovementSpeed;
        unsigned int movementSpeedTicksLeft;
        unsigned int swappedControlsTicksLeft;
    };

    /**
    * each entity that we want to do box collision on must have dimension component
    * for axis aligned bounding boxes
    * all measurement is measured from the center
    */
    struct BoxDimensionComponent : Component<BoxDimensionComponent> {
        BoxDimensionComponent(float xWidth, float yHeight, float zWidth) {
            halfDimension = glm::vec3(xWidth/2.0f, yHeight/2.0f, zWidth/2.0f);
        }
        glm::vec3 halfDimension;
    };

    struct EggHolderComponent : Component<EggHolderComponent> {
        EggHolderComponent(int holderId): holderId(holderId){
            isThrown = false;
            throwerId = holderId;
        }

        // holderId is the entity id of the player who hold the egg
        // if no one has the egg, default to INT_MIN
        int holderId;
        bool isThrown;
        int throwerId;
    };

    struct PlayerDataComponent : Component<PlayerDataComponent> {
        PlayerDataComponent(int teamID, PlayerType playerType, int points, int shootingCD) : teamID(teamID), playerType(playerType), points(points), shootingCD(shootingCD) {}
        int teamID;
        PlayerType playerType;
        int points;

        int shootingCD;
        time_t abilityTimer;
    };

    struct BallProjDataComponent : Component<BallProjDataComponent> {
        // Creates a generic inactive ball projectile
        BallProjDataComponent(BallProjType projType) {
            type = projType;// doesn't matter since not active
            active = false;
            collidedWithPlayer = false;
            creatorId = 0;
            collisionPlayerId = 0;
            exploded = false;
        }
        BallProjType type;
        bool active;
        bool collidedWithPlayer;
        unsigned int creatorId;
        unsigned int collisionPlayerId;
        bool exploded;
    };
    
    struct MeshCollisionComponent : Component<MeshCollisionComponent> {
        MeshCollisionComponent(std::vector<glm::vec3> collisionPoints, std::vector<int> groundPoints, bool active) : collisionPoints(collisionPoints), groundPoints(groundPoints), active(active) {}
        std::vector<glm::vec3> collisionPoints;
        bool active;
        std::vector<int> groundPoints;
    };

    struct CameraComponent : Component<CameraComponent> {
        CameraComponent() {
            distanceBehindPlayer = CAMERA_DISTANCE_BEHIND_PLAYER;
        }
        float distanceBehindPlayer;
        glm::vec3 direction;
    };

    struct SeasonAbilityStatusComponent : Component<SeasonAbilityStatusComponent> {
        SeasonAbilityStatusComponent() {
            coolDown = 0;
        }
        unsigned int coolDown;
    };

    struct LerpingComponent : Component<SeasonAbilityStatusComponent> {
        LerpingComponent(glm::vec3 start, glm::vec3 end) {
            curr = start;
            delta = (end - start) / LERP_DURATION_TICKS;
        }
        glm::vec3 curr;
        glm::vec3 delta;
        int t = LERP_DURATION_TICKS;
    };
    // disable egg snatching while learping (position.isLearping)
}
