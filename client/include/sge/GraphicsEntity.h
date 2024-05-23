//
// Created by benjx on 4/18/2024.
//

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <limits>
#include <random>
#include <bitset>
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
    private:
        // Not constants because we might want an environment object with a set "trajectory" / looped animations
        float pitch;
        float yaw;
        float roll;
        glm::vec3 position;
        // TODO: add velocity or whatever
//        glm::mat4 transform; // Entity's transformation matrix - if we add scaling/height n stuff
    protected:
        bool drawOutline = true; // Whether to draw outline
        // Add hitboxes here? idk
        bool castShadow = true; // Whether this entity should cast a shadow with the global light
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
        void drawShadow() const override;
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
     * Class for particle emitters
     */
    class ParticleEmitterEntity : BaseEntity {
    public:
        ParticleEmitterEntity();
        void draw() const override;
        void update() override;
        void emit(std::chrono::time_point<std::chrono::steady_clock> time);
        glm::vec3 spawnOrigin; // TODO: hook this up to whatever entity u want
    protected:
        std::bitset<MAX_PARTICLE_INSTANCE> activeParticles;
        size_t activeParticleCount; // Number of active particles

        bool active; // Whether the emitter is active

        std::vector<glm::vec3> positions;  // Particle position in world space
        std::vector<glm::vec3> velocities; // Particle velocity in world space

        std::vector<float> rotations;      // Particle rotation w.r.t. screen (angles, counter-clockwise)
        std::vector<float> angularVelocity;// Speed of rotation change
        glm::vec3 acceleration{};          // Particle acceleration per ms

        std::vector<float> blend;
        std::vector<std::chrono::time_point<std::chrono::steady_clock>> spawnTime; // Time-to-live for each particle, <= 0 for inactive particles
        std::chrono::time_point<std::chrono::steady_clock> lastUpdate;

        float spawnRate;    // Particles to spawn per tick
        float particleSize; // initial size of each particle (before any form of transformation)
        long long lifetime; // particle lifetime in milliseconds
        // can change to an array of these if we want emitters
        // to be able to have particles of multiple colors/types
        glm::vec4 initColor{};
        glm::vec4 endColor{};
    private:
        std::mt19937 generator;
        std::uniform_int_distribution<std::mt19937::result_type> dist;
        float sample();
    };
}
