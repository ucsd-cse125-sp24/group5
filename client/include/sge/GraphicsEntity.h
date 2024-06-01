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
        virtual void setAlternateTexture(bool allowAlternateTexture, int textureIdx);
        virtual void updateSeasons(bool _seasons);
    private:
        // Not constants because we might want an environment object with a set "trajectory" / looped animations
        float pitch;
        float yaw;
        float roll;
        glm::vec3 position;
        // TODO: add velocity or whatever
//        glm::mat4 transform; // Entity's transformation matrix - if we add scaling/height n stuff
    protected:
        bool alternateTextures = false;
        int textureIdx = 0;
        bool seasons = true;
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
     * Entity that emits particles :)
     *
     * Please inherit from this class or DiskParticleEmitterEntity for fire particles, snow particles, or whatever
     * so you don't always need to deal with all the available parameters.
     */
    class ParticleEmitterEntity : BaseEntity {
    public:
        ParticleEmitterEntity(float _spawnRate,
                              float _initParticleSize,
                              float _endParticleSize,
                              long long int _lifetime,
                              std::vector<float> _colorProbs,
                              std::vector<glm::vec4> _initColors,
                              std::vector<glm::vec4> _endColors,
                              glm::vec3 _spawnVelocityMultiplier,
                              glm::vec3 _spawnVelocityOffset,
                              float _angularVelocityMultiplier,
                              float _angularVelocityOffset,
                              glm::vec3 _acceleration,
                              glm::vec3 _position);

        ParticleEmitterEntity(float _spawnRate,
                              float _initParticleSize,
                              float _endParticleSize,
                              long long int _lifetime,
                              std::vector<float> _colorProbs,
                              std::vector<glm::vec4> _initColors,
                              std::vector<glm::vec4> _endColors,
                              glm::vec3 _spawnVelocityMultiplier,
                              glm::vec3 _spawnVelocityOffset,
                              float _angularVelocityMultiplier,
                              float _angularVelocityOffset,
                              glm::vec3 _acceleration,
                              size_t _positionIndex,
                              glm::vec3 _positionOffset);
        virtual void draw() const;
        void update() override;
        void burst();
        void explode();
        void setActive(bool active);
    protected:
        void emit(long long time, int count, bool explode);
        float sample();
        virtual glm::vec3 sampleParticlePosition();
        glm::vec3 emitterPosition{};
    private:
        std::bitset<MAX_PARTICLE_INSTANCE> activeParticles;
        size_t activeParticleCount; // Number of active particles

        bool isActive; // Whether the emitter is active
        bool dynamic;
        size_t positionIndex;
        glm::vec3 positionOffset;

        std::vector<glm::vec3> positions;  // Particle position in world space

        std::vector<glm::vec3> velocities; // Particle velocity in world space
        glm::vec3 spawnVelocityMultiplier;
        glm::vec3 spawnVelocityOffset;
        glm::vec3 acceleration;          // Particle acceleration per ms

        std::vector<float> rotations;      // Particle rotation w.r.t. screen (angles, counter-clockwise)
        std::vector<float> angularVelocities;// Speed of rotation change
        float spawnAngularVelocityMultiplier;
        float spawnAngularVelocityOffset;

        std::vector<float> blend;
        std::vector<long long> spawnTime; // Time-to-live for each particle, <= 0 for inactive particles
        long long lastUpdate;

        float spawnRate;    // Particles to spawn per tick
        float initParticleSize; // initial size of each particle (before any form of transformation)
        float endParticleSize; // final size of each particle (before any form of transformation)

        long long lifetime; // particle lifetime in milliseconds
        glm::vec4 endColor;

        std::vector<int> colorIdx;
        std::vector<float> colorProbs;
        std::vector<glm::vec4> initColors;
        std::vector<glm::vec4> endColors;

        constexpr static float EXPLOSION_VELOCITY_MULTIPLIER = 5.0f;
    };

    /**
     * Disk particle emitter, mostly meant for ambience
     * or potential splash effects (see Minecraft lingering potions)
     *
     * Please inherit from this class or ParticleEmitterEntity for fire particles, snow particles, or whatever
     * so you don't always need to deal with all the available parameters.
     */
    class DiskParticleEmitterEntity : public ParticleEmitterEntity {
    public:
        DiskParticleEmitterEntity(float spawnRate,
                              float initParticleSize,
                              float endParticleSize,
                              long long int lifetime,
                              std::vector<float> colorProbs,
                              std::vector<glm::vec4> initColors,
                              std::vector<glm::vec4> endColors,
                              glm::vec3 spawnVelocityMultiplier,
                              glm::vec3 spawnVelocityOffset,
                              float angularVelocityMultiplier,
                              float angularVelocityOffset,
                              glm::vec3 acceleration,
                              glm::vec3 position,
                              float radius);

        DiskParticleEmitterEntity(float spawnRate,
                              float initParticleSize,
                              float endParticleSize,
                              long long int lifetime,
                              std::vector<float> colorProbs,
                              std::vector<glm::vec4> initColors,
                              std::vector<glm::vec4> endColors,
                              glm::vec3 spawnVelocityMultiplier,
                              glm::vec3 spawnVelocityOffset,
                              float angularVelocityMultiplier,
                              float angularVelocityOffset,
                              glm::vec3 acceleration,
                              size_t positionIndex,
                              glm::vec3 positionOffset,
                              float radius);
    protected:
        glm::vec3 sampleParticlePosition() override;
    private:
        float radius;
    };

    extern std::mt19937 generator; // Random number generator
    extern std::uniform_int_distribution<std::mt19937::result_type> dist;
}
