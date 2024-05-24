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
 * Draw the current particles to the screen
 */
void sge::ParticleEmitterEntity::draw() const {
    if (!isActive && activeParticleCount == 0) return;
    static ParticleEmitterState state;
    state.colors.clear();
    state.transforms.clear();
    state.baseParticleSize = particleSize;
    for (int i = 0; i < MAX_PARTICLE_INSTANCE; i++) {
        if (!activeParticles[i]) {
            continue;
        }
        state.transforms.push_back(glm::rotate(glm::translate(glm::mat4(1), positions[i]), glm::radians(rotations[i]), glm::vec3(0, 0, 1)));
        state.colors.push_back(glm::mix(initColors[colorIdx[i]], endColors[colorIdx[i]], blend[i]));
    }
    emitters[0]->render(state, state.colors.size());
}

/**
 * Update all particle positions and velocities
 */
void sge::ParticleEmitterEntity::update() {
    if (!isActive && activeParticleCount == 0) return;
    if (dynamic) {
        emitterPosition = clientGame->positions[positionIndex];
    }
    long long time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    long long delta = time - lastUpdate;
    // mult is the fraction of a game tick that has occured since the last particle update
    float mult = (float)delta / (float)33; // 33 is interval between game ticks in ms, we don't have a constant for that for some reason???
    for (int i = 0; i < MAX_PARTICLE_INSTANCE; i++) {
        if (!activeParticles[i]) {
            continue;
        }
        positions[i] += mult * velocities[i];
        rotations[i] += mult * angularVelocities[i];
        velocities[i] += mult * acceleration;
        // Time since this particle was emitted
        long long ms = time - spawnTime[i];
        blend[i] = (float)ms / (float)lifetime;

        if (blend[i] > 1) {
            activeParticles[i] = false;
            activeParticleCount--;
        }
    }
    lastUpdate = time;

    // Emit more particles if emitter is active
    if (isActive) {
        float toSpawn = mult * spawnRate;
        int spawn = (int)toSpawn;
        float probSpawn = std::fmod(toSpawn, 1);
        if (sample() < probSpawn) {
            spawn++;
        }
        emit(time, spawn);
    }
}

/**
 * Sample from the uniform distribution between 0 and 1
 * @return
 */
float sge::ParticleEmitterEntity::sample() {
    return (float)dist(generator) / (float)UINT32_MAX;
}

/**
 * Set whether the emitter emits particles
 * @param active
 */
void sge::ParticleEmitterEntity::setActive(bool active) {
    isActive = active;
}

/**
 *
 * @param spawnRate
 * @param particleSize
 * @param lifetime
 * @param initColor
 * @param endColor
 * @param spawnVelocityMultiplier
 * @param spawnVelocityOffset
 * @param angularVelocityMultiplier
 * @param angularVelocityOffset
 * @param acceleration
 * @param position
 */
sge::ParticleEmitterEntity::ParticleEmitterEntity(float spawnRate,
                                                  float particleSize,
                                                  long long int lifetime,
                                                  std::vector<float> colorProbs,
                                                  std::vector<glm::vec4> initColors,
                                                  std::vector<glm::vec4> endColors,
                                                  glm::vec3 spawnVelocityMultiplier,
                                                  glm::vec3 spawnVelocityOffset,
                                                  float angularVelocityMultiplier,
                                                  float angularVelocityOffset,
                                                  glm::vec3 acceleration,
                                                  glm::vec3 position) {
    generator.seed(std::random_device()());
    activeParticles.reset();
    activeParticleCount = 0;

    positions.assign(MAX_PARTICLE_INSTANCE, glm::vec3(0));
    velocities.assign(MAX_PARTICLE_INSTANCE, glm::vec3(0));

    rotations.assign(MAX_PARTICLE_INSTANCE, 0.0f);
    angularVelocities.assign(MAX_PARTICLE_INSTANCE, 0.0f);

    blend.assign(MAX_PARTICLE_INSTANCE, 0);
    spawnTime.assign(MAX_PARTICLE_INSTANCE, 0);
    lastUpdate = 0;

    dynamic = false;
    positionIndex = -1;

    this->spawnRate = spawnRate;
    this->particleSize = particleSize;
    this->lifetime = lifetime;

    this->colorProbs = colorProbs;
    this->initColors = initColors;
    this->endColors = endColors;
    colorIdx.assign(MAX_PARTICLE_INSTANCE, -1);

    this->spawnVelocityMultiplier = spawnVelocityMultiplier;
    this->spawnVelocityOffset = spawnVelocityOffset;

    this->spawnAngularVelocityMultiplier = angularVelocityMultiplier;
    this->spawnAngularVelocityOffset = angularVelocityOffset;
    this->acceleration = acceleration;
    this->emitterPosition = position;
}

/**
 *
 * @param spawnRate
 * @param particleSize
 * @param lifetime
 * @param initColor
 * @param endColor
 * @param spawnVelocityMultiplier
 * @param spawnVelocityOffset
 * @param angularVelocityMultiplier
 * @param angularVelocityOffset
 * @param acceleration
 * @param positionIndex
 * @param positionOffset
 */
sge::ParticleEmitterEntity::ParticleEmitterEntity(float spawnRate,
                                                  float particleSize,
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
                                                  glm::vec3 positionOffset) {
    generator.seed(std::random_device()());
    activeParticles.reset();
    activeParticleCount = 0;

    positions.assign(MAX_PARTICLE_INSTANCE, glm::vec3(0));
    velocities.assign(MAX_PARTICLE_INSTANCE, glm::vec3(0));

    rotations.assign(MAX_PARTICLE_INSTANCE, 0.0f);
    angularVelocities.assign(MAX_PARTICLE_INSTANCE, 0.0f);

    blend.assign(MAX_PARTICLE_INSTANCE, 0);
    spawnTime.assign(MAX_PARTICLE_INSTANCE, 0);

    dynamic = true;
    this->positionIndex = positionIndex;

    this->spawnRate = spawnRate;
    this->particleSize = particleSize;
    this->lifetime = lifetime;

    this->colorProbs = colorProbs;
    this->initColors = initColors;
    this->endColors = endColors;
    colorIdx.assign(MAX_PARTICLE_INSTANCE, -1);

    this->spawnVelocityMultiplier = spawnVelocityMultiplier;
    this->spawnVelocityOffset = spawnVelocityOffset;

    this->spawnAngularVelocityMultiplier = angularVelocityMultiplier;
    this->spawnAngularVelocityOffset = angularVelocityOffset;
    this->acceleration = acceleration;
    this->emitterPosition = positionOffset;
}

/**
 * Emit count number of particles if there is a slot available
 * @param time Current system time for keeping track of particle lifetimes
 * @param count Number of particles to emit
 */
void sge::ParticleEmitterEntity::emit(long long time, int count) {
    int spawned = 0; // Number of particles spawned so far
    for (int i = 0; i < MAX_PARTICLE_INSTANCE && spawned < count; i++) {
        if (activeParticles[i]) continue;

        float randColor = sample();
        float sum = 0;

        if (colorProbs.size() == 1) {
            colorIdx[i] = 0;
        } else {
            // Pick a color, any color :)
            for (int j = 0; j < initColors.size(); j++) {
                sum += colorProbs[j];
                if (randColor <= sum + 0.001) {
                    colorIdx[i] = j;
                    break;
                }
            }
        }

        glm::vec3 randVelocity(sample(), sample(), sample());

        positions[i] = emitterPosition; // TODO: change this to allow for other types of emitters e.g. square emitters
        velocities[i] = spawnVelocityMultiplier * (randVelocity + spawnVelocityOffset);
        spawnTime[i] = time;
        rotations[i] = sample() * 90; // 90 for 90 degrees
        angularVelocities[i] = spawnAngularVelocityMultiplier * (sample() + spawnAngularVelocityOffset);
        activeParticles[i] = true;
        spawned++;
    }
}
