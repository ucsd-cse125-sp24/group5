#pragma once

#include <glm/glm.hpp>
#include "GameConstants.h"
#include <vector>
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
        MovementRequestComponent(bool forwardRequested, bool backwardRequested, bool leftRequested, bool rightRequested, bool jumpRequested, float pitch, float yaw)
            : forwardRequested(forwardRequested), backwardRequested(backwardRequested), leftRequested(leftRequested), rightRequested(rightRequested), 
                jumpRequested(jumpRequested), pitch(pitch), yaw(yaw) {
        }
        bool forwardRequested, backwardRequested, leftRequested, rightRequested, jumpRequested;
        float yaw, pitch;
        glm::vec3 forwardDirection;
        glm::vec3 rightwardDirection;
    };


    struct HealthComponent : Component<HealthComponent> {
        HealthComponent(int healthPoint) : healthPoint(healthPoint) {
        }
        int healthPoint;
    };

    /**
    * each entity that we want to do box collision on must have dimension component
    * we assume that these entity are cylinder-shaped
    * all measurement is measured from the center
    */
    struct BoxDimensionComponent : Component<BoxDimensionComponent> {
        BoxDimensionComponent(float xWidth, float yHeight, float zWidth) {
            halfDimension = glm::vec3(xWidth/2.0f, yHeight/2.0f, zWidth/2.0f);
        }
        glm::vec3 halfDimension;
    };

    struct EggHolderComponent : Component<EggHolderComponent> {
        EggHolderComponent(int holderId): holderId(holderId){}

        // holderId is the entity id of the player who hold the egg
        // if no one has the egg, default to MIN_INT
        int holderId;
    };

    struct PlayerDataComponent : Component<PlayerDataComponent> {
        PlayerDataComponent(int teamID, PlayerType playerType, int points, time_t shootingTimer) : teamID(teamID), playerType(playerType), points(points), shootingTimer(shootingTimer) {}
        int teamID;
        PlayerType playerType;
        int points;

        time_t shootingTimer;
        time_t abilityTimer;
    };
    
    struct MeshCollisionComponent : Component<MeshCollisionComponent> {
        MeshCollisionComponent(std::vector<glm::vec3> collisionPoints, std::vector<int> groundPoints, float rayLength) : collisionPoints(collisionPoints), groundPoints(groundPoints), rayLength(rayLength) {}
        std::vector<glm::vec3> collisionPoints;
        std::vector<int> groundPoints;
        float rayLength; // TODO: 1 for player, ~70 for bullets
    };

    struct CameraComponent : Component<CameraComponent> {
        CameraComponent() {
            distanceBehindPlayer = CAMERA_DISTANCE_BEHIND_PLAYER;
        }
        float distanceBehindPlayer;
        glm::vec3 direction;
    };
}
