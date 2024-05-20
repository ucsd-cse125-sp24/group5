//
// Created by benjx on 4/18/2024.
//

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include "GraphicsGeometry.h"

namespace sge {

    class BaseEntity {
    public:
        BaseEntity() = default;
        virtual void draw() const = 0;
        virtual void update() = 0;
    protected:
    };

    /**
     * Class that wraps position and model information in a
     * memory-usage friendly package
     *
     * Entity that has a 3d model
     */
    class ModelEntityState : BaseEntity {
    public:
        ModelEntityState(size_t modelIndex);
        ModelEntityState(size_t modelIndex, glm::vec3 position);
        ModelEntityState(size_t modelIndex, glm::vec3 position, float yaw, float pitch, float roll);
        // Draw this element to the screen
        virtual void draw() const override;
        virtual void drawShadow() const;
        virtual void update() override;
        virtual void updateOutline(bool outline);
        virtual void updateShadow(bool shadow);
    protected:
        bool drawOutline; // Whether to draw outline
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
        bool castShadow; // Whether this entity should cast a shadow with the global light
        const size_t modelIndex; // This entity's index in GraphicsGeometry.h's model array
    };

    /**
     * Entity state for dynamic entities, e.g. players
     *
     * For dynamic entities with 3d models
     */
    class DynamicModelEntityState : public ModelEntityState {
    public:
        DynamicModelEntityState(size_t modelIndex, size_t positionIndex);
        void draw() const override;
        virtual void drawShadow() const override;
        void update() override;
        void setAnimation(unsigned int animationId);
    protected:
        void startAnimation(unsigned int animationIndex);
        void stopAnimation();
        const size_t positionIndex;
        ModelPose currPose;
        int currentAnimationIndex; // Which animation are we currently displaying? -1 for no animation
        float animationTime; // time within the animation loop (ranges from 0 to the animation's duration
        std::chrono::high_resolution_clock::time_point animationStartTime; // what was the computer's time when we last started the animation?
    };

    /**
     * Class for individual particle types
     */
    class Particle : BaseEntity {

    };
}
