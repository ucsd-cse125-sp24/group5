#pragma once

#include <glm/glm.hpp>

namespace bge {

    struct ComponentCounter {
        inline static int familyCounter = 0;
    };

    template <typename ComponentType>
    struct Component {
        static inline int family() {
            static int family = ComponentCounter::familyCounter++;
            return family;
        }
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
            : forwardRequested(forwardRequested), backwardRequested(backwardRequested), leftRequested(leftRequested), rightRequested(rightRequested), jumpRequested(jumpRequested), pitch(pitch), yaw(yaw) {
        }
        bool forwardRequested, backwardRequested, leftRequested, rightRequested, jumpRequested;
        float yaw, pitch;
    };

}
