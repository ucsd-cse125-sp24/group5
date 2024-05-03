#pragma once

#include <glm/glm.hpp>
#include "GameConstants.h"

namespace bge {

    template <typename ComponentType>
    struct Component {

    };

    struct PositionComponent: Component<PositionComponent> {
        PositionComponent(float x, float y, float z) {
            position = glm::vec3(x,y,z);
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
            : forwardRequested(forwardRequested), backwardRequested(backwardRequested), leftRequested(leftRequested), rightRequested(rightRequested), jumpRequested(jumpRequested), pitch(pitch), yaw(yaw) {
        }
        bool forwardRequested, backwardRequested, leftRequested, rightRequested, jumpRequested;
        float yaw, pitch;
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
    };
}
