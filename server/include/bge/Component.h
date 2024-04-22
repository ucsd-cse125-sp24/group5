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

    struct MovementRequestComponent : Component<MovementRequestComponent> {
        MovementRequestComponent(bool forward, bool backward, bool left, bool right, bool jump, float forwardX, float forwardY, float forwardZ)
            : forward(forward), backward(backward), left(left), right(right), jump(jump) {
            forwardDirection = glm::vec3(forwardX, forwardY, forwardZ);
        }
        bool forward, backward, left, right, jump;
        glm::vec3 forwardDirection;
    };

}
