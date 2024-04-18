#pragma once

#include <glm/glm.hpp>

namespace bge {
    struct ComponentCounter {
        static int familyCounter;
    };

    template <typename ComponentType>
    struct Component {
        static inline int family() {
            static int family = ComponentCounter::familyCounter++;
            return family;
        }
    };

    template <typename C>
    static int GetComponentFamily() {
        return Component<typename std::remove_const<C>::type>::family();
    }

    struct PositionComponent: Component<PositionComponent> {
        PositionComponent(float x, float y, float z) {
            position = glm::vec3(x,y,z);
        }
        glm::vec3 position;
    };

}
