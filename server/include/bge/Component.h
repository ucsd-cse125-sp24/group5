#pragma once

#include <glm/glm.hpp>

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
    struct DimensionComponent : Component<DimensionComponent> {
        DimensionComponent(int height, int radius) : height(height), radius(radius) {}
        int height;
        int radius;
    };

    struct EggHolderComponent : Component<EggHolderComponent> {
        EggHolderComponent(int holderId): holderId(holderId){}

        // holderId is the entity id of the player who hold the egg
        // if no one has the egg, default to MIN_INT
        int holderId;
    };
    
}
