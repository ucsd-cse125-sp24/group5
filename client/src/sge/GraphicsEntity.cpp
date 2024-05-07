//
// Created by benjx on 4/18/2024.
//
#include "sge/GraphicsEntity.h"
#include "Client.h"

sge::EntityState::EntityState(size_t modelIndex) : modelIndex(modelIndex) {
    position = glm::vec3(0.0f);
    pitch = 0.0f;
    yaw = 0.0f;
    roll = 0.0f;
}

sge::EntityState::EntityState(size_t modelIndex, glm::vec3 position) : modelIndex(modelIndex), position(position) {
    pitch = 0.0f;
    yaw = 0.0f;
    roll = 0.0f;
}

sge::EntityState::EntityState(size_t modelIndex, glm::vec3 position, float yaw, float pitch, float roll) : modelIndex(modelIndex), position(position), yaw(yaw), pitch(pitch), roll(roll) {
    // Initialization handled in constructor initializer list
}

/**
 * PRECONDITION: models have already been loaded
 * Draws the entity on the screen
 */
void sge::EntityState::draw() const {
    // TODO: add support for server-side roll? Maybe add pitch too here
    models[modelIndex]->render(position, yaw);
}

void sge::EntityState::update() {}

sge::DynamicEntityState::DynamicEntityState(size_t modelIndex, size_t positionIndex) : EntityState(modelIndex), positionIndex(positionIndex) {
    currentAnimationIndex = 0; // Which animation are we currently displaying? -1 for no animation
    animationTime = 0; // time within the animation loop (ranges from 0 to the animation's duration)
    animationStartTime = std::chrono::high_resolution_clock::now();
    currPose = models[modelIndex]->emptyModelPose();
}

/**
 * Draw entity to screen
 */
void sge::DynamicEntityState::draw() const {
    models[modelIndex]->renderPose(clientGame->positions[positionIndex], clientGame->yaws[positionIndex], currPose);
}

void sge::DynamicEntityState::update() {
    // std::cout << "updating animation time\n";
    if (currentAnimationIndex == -1) {
        animationTime = 0;
    }
    else {
        auto now = std::chrono::high_resolution_clock::now();
        auto timeSinceStart = now - animationStartTime;
        long long milliSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceStart).count();
        animationTime = models[modelIndex]->timeToAnimationTick(milliSinceStart,currentAnimationIndex);
    }
    models[modelIndex]->animationPose(currentAnimationIndex, animationTime, currPose);
    animationTime++;
}


