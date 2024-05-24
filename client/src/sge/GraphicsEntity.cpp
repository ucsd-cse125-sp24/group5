//
// Created by benjx on 4/18/2024.
//
#include "sge/GraphicsEntity.h"
#include "Client.h"

/**
 * Create a new entity to be rendered with the specified model
 * @param modelIndex
 */
sge::ModelEntityState::ModelEntityState(size_t modelIndex) : modelIndex(modelIndex) {
    position = glm::vec3(0.0f);
    pitch = 0.0f;
    yaw = 0.0f;
    roll = 0.0f;
    drawOutline = true;
    castShadow = true;
}

/**
 * Create a new entity to be rendered with the specified model and position
 * @param modelIndex
 * @param position
 */
sge::ModelEntityState::ModelEntityState(size_t modelIndex, glm::vec3 position) : modelIndex(modelIndex), position(position) {
    pitch = 0.0f;
    yaw = 0.0f;
    roll = 0.0f;
    drawOutline = true;
    castShadow = true;
}

/**
 * Create a new entity to be rendered with the specified model and pose
 * @param modelIndex
 * @param position
 * @param yaw
 * @param pitch
 * @param roll
 */
sge::ModelEntityState::ModelEntityState(size_t modelIndex, glm::vec3 position, float yaw, float pitch, float roll) : modelIndex(modelIndex), position(position), yaw(yaw), pitch(pitch), roll(roll) {
    // Initialization handled in constructor initializer list
    drawOutline = true;
    castShadow = true;
}

/**
 * PRECONDITION: models have already been loaded
 * Draws the entity on the screen
 */
void sge::ModelEntityState::draw() const {
    // TODO: add support for server-side roll? Maybe add pitch too here
    models[modelIndex]->render(position, yaw, false, drawOutline);
}

/**
 * Update entity state
 */
void sge::ModelEntityState::update() {}

/**
 * PRECONDITION: models have already been loaded
 * Draws the entity to the shadowmap
 */
void sge::ModelEntityState::drawShadow() const {
    if (!castShadow) return;
    models[modelIndex]->render(position, yaw, true, true);
}

/**
 * Set whether to draw outlines for this entity
 * @param outline
 */
void sge::ModelEntityState::updateOutline(bool outline) {
    drawOutline = outline;
}

/**
 * Set whether this entity casts a shadow with the global light
 * @param shadow
 */
void sge::ModelEntityState::updateShadow(bool shadow) {
    castShadow = shadow;
}

/**
 * Create a new dynamic entity (an entity with changing state)
 * with the specified model and position index into the position vector (in clientgame.h)
 * @param modelIndex
 * @param positionIndex
 */
sge::DynamicModelEntityState::DynamicModelEntityState(size_t modelIndex, size_t positionIndex) : ModelEntityState(modelIndex), positionIndex(positionIndex) {
    currentAnimationIndex = 0; // Which animation are we currently displaying? -1 for no animation
    animationTime = 0; // time within the animation loop (ranges from 0 to the animation's duration)
    animationStartTime = std::chrono::high_resolution_clock::now();
    currPose = models[modelIndex]->emptyModelPose();
    currentAnimationIndex = -1;
}

/**
 * Draw entity to screen
 */
void sge::DynamicModelEntityState::draw() const {
    if (models[modelIndex]->isAnimated()) {
        models[modelIndex]->renderPose(clientGame->positions[positionIndex], clientGame->yaws[positionIndex], currPose,
                                       false, drawOutline);
    } else {
        models[modelIndex]->render(clientGame->positions[positionIndex], clientGame->yaws[positionIndex], false, drawOutline);
    }
}

/**
 * Update dynamic entity animation state
 */
void sge::DynamicModelEntityState::update() {
    auto now = std::chrono::high_resolution_clock::now();
    auto timeSinceStart = now - animationStartTime;
    long long milliSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceStart).count();
    if (models[modelIndex]->isAnimated()) {
        animationTime = models[modelIndex]->timeToAnimationTick(milliSinceStart,currentAnimationIndex);
        models[modelIndex]->animationPose(currentAnimationIndex, animationTime, currPose);
    }
}

/**
 * Start animation routine
 * @param animationId
 */
void sge::DynamicModelEntityState::startAnimation(unsigned int animationId) {
    currentAnimationIndex = animationId;
    animationStartTime = std::chrono::high_resolution_clock::now();
}

/**
 * Change curent entity's animation to specified animation id
 * Does nothing if model has no animation
 * @param animationId
 */
void sge::DynamicModelEntityState::setAnimation(unsigned int animationId) {
    if (animationId == SHOOTING) {
        // restart shooting animation (don't put your gun down)
        startAnimation(SHOOTING);
        return;
    }
    if (animationId == currentAnimationIndex || !models[modelIndex]->isAnimated()) {
        return;
    } else {
        startAnimation(animationId);
    }
}
/*
 * PRECONDITION: models have already been loaded
 * Draws the entity to the shadowmap
 */
void sge::DynamicModelEntityState::drawShadow() const {
    if (!castShadow) return;
    if (models[modelIndex]->isAnimated()) {
        models[modelIndex]->renderPose(clientGame->positions[positionIndex], clientGame->yaws[positionIndex], currPose,
                                       true, true);
    } else {
        models[modelIndex]->render(clientGame->positions[positionIndex], clientGame->yaws[positionIndex], true, true);
    }
}

/**
 * Create a new particle emitter entity
 * Each entity maintains its own state on stuff like particle positions
 * colors, etc.
 */
sge::ParticleEmitterEntity::ParticleEmitterEntity() {
    generator.seed(std::random_device()());

    activeParticles.reset();
    activeParticleCount = 0;

    positions.assign(MAX_PARTICLE_INSTANCE, glm::vec3(0));
    velocities.assign(MAX_PARTICLE_INSTANCE, glm::vec3(0));

    rotations.assign(MAX_PARTICLE_INSTANCE, 0.0f);
    angularVelocity.assign(MAX_PARTICLE_INSTANCE, 0.0f);

    blend.assign(MAX_PARTICLE_INSTANCE, 0);
    spawnTime.assign(MAX_PARTICLE_INSTANCE, std::chrono::high_resolution_clock::time_point());

    spawnRate = 10;
    particleSize = 0.2f;
    initColor = glm::vec4(1, 0, 0, 1);
    endColor = glm::vec4(1, 1, 0, 0);
    acceleration = glm::vec3(0, -0.005, 0);
    lifetime = 2000;
}

/**
 * Draw the current particles to the screen
 */
void sge::ParticleEmitterEntity::draw() const {
    static ParticleEmitterState state;
    state.colors.clear();
    state.transforms.clear();
    state.baseParticleSize = particleSize;
    for (int i = 0; i < MAX_PARTICLE_INSTANCE; i++) {
        if (!activeParticles[i]) {
            continue;
        }
        state.transforms.push_back(glm::rotate(glm::translate(glm::mat4(1), positions[i]), glm::radians(rotations[i]), glm::vec3(0, 0, 1)));
        state.colors.push_back(glm::mix(initColor, endColor, blend[i]));
    }
    emitters[0]->render(state, state.colors.size());
}

/**
 * Update all particle positions and velocities
 */
void sge::ParticleEmitterEntity::update() {
    std::chrono::time_point<std::chrono::steady_clock> time = std::chrono::high_resolution_clock::now();
    long long delta = std::chrono::duration_cast<std::chrono::milliseconds>(time - lastUpdate).count();
    // mult is the fraction of a game tick that has occured since the last particle update
    float mult = (float)delta / (float)33; // 33 is interval between game ticks in ms, we don't have a constant for that for some reason???
    for (int i = 0; i < MAX_PARTICLE_INSTANCE; i++) {
        if (!activeParticles[i]) {
            continue;
        }
        positions[i] += mult * velocities[i];
        rotations[i] += mult * angularVelocity[i];
        velocities[i] += mult * acceleration;
        // Time since this particle was emitted
        long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(time - spawnTime[i]).count();
        blend[i] = (float)ms / (float)lifetime;

        if (blend[i] > 1) {
            activeParticles[i] = false;
            activeParticleCount--;
        }
    }
    lastUpdate = time;

    float toSpawn = mult * spawnRate;
    int spawn = (int)toSpawn;
    float probSpawn = std::fmod(toSpawn, 1);
    if (sample() < probSpawn) {
        spawn++;
    }
    for (int i = 0; i < spawn; i++) {
        emit(time);
    }
}

/**
 * Emit a particle (if possible)
 */
void sge::ParticleEmitterEntity::emit(std::chrono::time_point<std::chrono::steady_clock> time) {
    for (int i = 0; i < MAX_PARTICLE_INSTANCE; i++) {
        if (activeParticles[i]) continue;

        float vx = 0.1 * (sample() - 0.5);
        float vy = 0.5 * (sample());
        float vz = 0.1 * (sample() - 0.5);

        positions[i] = spawnOrigin;
        velocities[i] = glm::vec3(vx, vy, vz);
        spawnTime[i] = time;
        rotations[i] = sample() * 90;
        angularVelocity[i] = sample() - 0.5f;
        activeParticles[i] = true;
        break;
    }
}

/**
 * Sample from the uniform distribution between 0 and 1
 * @return
 */
float sge::ParticleEmitterEntity::sample() {
    return (float)dist(generator) / (float)UINT32_MAX;
}
