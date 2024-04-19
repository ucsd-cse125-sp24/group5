//
// Created by benjx on 4/18/2024.
//

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace sge {
    /**
     * Class that wraps position and model information in a
     * memory-usage friendly package
     */
    class EntityState {
    public:
        EntityState(size_t modelIndex);
        EntityState(size_t modelIndex, glm::vec3 position);
        EntityState(size_t modelIndex, glm::vec3 position, float yaw, float pitch, float roll);
        // Draw this element to the screen
        virtual void draw() const;
    private:
        // Not constants because we might want an environment object with a set "trajectory" / looped animations
        float pitch;
        float yaw;
        float roll;
        glm::vec3 position;
        // TODO: add velocity or whatever
//        glm::mat4 transform; // Entity's transformation matrix - if we add scaling/height n stuff
    protected:
        // Add hitboxes here? idk
        const size_t modelIndex; // This entity's index in GraphicsGeometry.h's model array
    };

    /**
     * Entity state for dynamic entities, e.g. players
     */
    class DynamicEntityState : public EntityState {
    public:
        DynamicEntityState(size_t modelIndex, size_t positionIndex);
        void draw() const override;
    protected:
        const size_t positionIndex;
    };

    class AnimatedEntityState : public EntityState {
    public:
        // Add extra state information for animation stuff idk
    };
}
