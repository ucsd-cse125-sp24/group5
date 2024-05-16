#pragma once

#include <glm/glm.hpp>
#include "GameConstants.h"
#include <vector>

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
    };

    struct VelocityComponent : Component<VelocityComponent> {
        VelocityComponent(float vx, float vy, float vz) {
            velocity = glm::vec3(vx, vy, vz);
        }
        glm::vec3 velocity;
        bool onGround;
    };

    struct JumpInfoComponent : Component<JumpInfoComponent> {
        JumpInfoComponent(int doubleJumpUsed, bool jumpHeld) : doubleJumpUsed(doubleJumpUsed), jumpHeld(jumpHeld) {}
        int doubleJumpUsed;
        bool jumpHeld;
    };

    struct MovementRequestComponent : Component<MovementRequestComponent> {
        MovementRequestComponent(bool forwardRequested, bool backwardRequested, bool leftRequested, bool rightRequested, bool jumpRequested, bool throwEggRequested, float pitch, float yaw)
            : forwardRequested(forwardRequested), backwardRequested(backwardRequested), leftRequested(leftRequested), rightRequested(rightRequested), 
                jumpRequested(jumpRequested), throwEggRequested(throwEggRequested), pitch(pitch), yaw(yaw) {
        }
        bool forwardRequested, backwardRequested, leftRequested, rightRequested, jumpRequested, throwEggRequested;
        float yaw, pitch;
        glm::vec3 forwardDirection;
    };


    struct HealthComponent : Component<HealthComponent> {
        HealthComponent(int healthPoint) : healthPoint(healthPoint) {
        }
        int healthPoint;
    };

    struct SpeedChangeComponent : Component<SpeedChangeComponent> {
        SpeedChangeComponent(float alternateMovementSpeed, unsigned int ticksLeft) : alternateMovementSpeed(alternateMovementSpeed), ticksLeft(ticksLeft) {}
        float alternateMovementSpeed;
        unsigned int ticksLeft;
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
        PlayerDataComponent(int teamID, PlayerType playerType, int points) : teamID(teamID), playerType(playerType), points(points) {}
        int teamID;
        PlayerType playerType;
        int points;
    };
    
    struct MeshCollisionComponent : Component<MeshCollisionComponent> {
        MeshCollisionComponent(std::vector<glm::vec3> collisionPoints, std::vector<int> groundPoints) : collisionPoints(collisionPoints), groundPoints(groundPoints) {}
        std::vector<glm::vec3> collisionPoints;
        std::vector<int> groundPoints;
    };

    struct CameraComponent : Component<CameraComponent> {
        CameraComponent() {
            distanceBehindPlayer = CAMERA_DISTANCE_BEHIND_PLAYER;
        }
        float distanceBehindPlayer;
        glm::vec3 direction;
    };
}
