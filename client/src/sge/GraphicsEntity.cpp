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

void sge::DynamicModelEntityState::drawShadow() const {
    if (!castShadow) return;
    if (models[modelIndex]->isAnimated()) {
        models[modelIndex]->renderPose(clientGame->positions[positionIndex], clientGame->yaws[positionIndex], currPose,
                                       true, true);
    } else {
        models[modelIndex]->render(clientGame->positions[positionIndex], clientGame->yaws[positionIndex], true, true);
    }
}

sge::ParticleEmitterEntity::ParticleEmitterEntity() {
    activeParticles.reset();
    activeParticleCount = 0;

    positions.assign(MAX_PARTICLE_INSTANCE, glm::vec3(0));
    velocities.assign(MAX_PARTICLE_INSTANCE, glm::vec3(0));

    rotations.assign(MAX_PARTICLE_INSTANCE, 0.0f);
    angularVelocity.assign(MAX_PARTICLE_INSTANCE, 0.0f);

    blend.assign(MAX_PARTICLE_INSTANCE, 0);
    spawnTime.assign(MAX_PARTICLE_INSTANCE, std::chrono::high_resolution_clock::time_point());

    spawnRate = 10;
    particleSize = 1.0f;
    initColor = glm::vec4(1, 0, 0, 1);
    endColor = glm::vec4(1, 1, 0, 0);
    acceleration = glm::vec3(0, -0.05, 0);
    lifetime = 10000;
}

void sge::ParticleEmitterEntity::draw() const {
    static ParticleEmitterState state;
    state.colors.clear();
    state.transforms.clear();
    state.baseParticleSize = particleSize;
    for (int i = 0; i < MAX_PARTICLE_INSTANCE; i++) {
        if (!activeParticles[i]) {
            continue;
        }
        state.transforms.push_back(glm::translate(glm::rotate(glm::mat4(1), glm::radians(rotations[i]), glm::vec3(0, 0, 1)), positions[i]));
        state.colors.push_back(glm::mix(initColor, endColor, 0.5));
    }
    emitters[0]->render(state, state.colors.size());
}

void sge::ParticleEmitterEntity::update() {
    std::chrono::time_point<std::chrono::steady_clock> time = std::chrono::high_resolution_clock::now();
    long long delta = std::chrono::duration_cast<std::chrono::milliseconds>(time - lastUpdate).count();
    float mult = (float)delta / (float)33; // 33 is interval between ticks in ms, we don't have a constant for that for some reason???
    for (int i = 0; i < MAX_PARTICLE_INSTANCE; i++) {
        if (!activeParticles[i]) {
            continue;
        }
        positions[i] += 0.1f * mult * velocities[i];
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

    // hard coded in for now
    emit();
}

/**
 * Emit a particle (if possible)
 */
void sge::ParticleEmitterEntity::emit() {
    for (int i = 0; i < MAX_PARTICLE_INSTANCE; i++) {
        if (activeParticles[i]) continue;

        float vx = dist(generator);
        float vy = dist(generator) + 5;
        float vz = dist(generator);

        positions[i] = spawnOrigin;
        velocities[i] = 0.01f * glm::vec3(vx, vy, vz);

        rotations[i] = dist(generator) * 90;
        angularVelocity[i] = dist(generator);
        activeParticles[i] = true;
        break;
    }
}
