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
    animationTime = 0;
}

/**
 * Draw entity to screen
 */
void sge::DynamicEntityState::draw() const {
    ModelPose pose = models[modelIndex]->animationPose(0, animationTime);
    models[modelIndex]->renderPose(clientGame->positions[positionIndex], clientGame->yaws[positionIndex], pose);
}

void sge::DynamicEntityState::update() {
    // std::cout << "updating animation time\n";
    animationTime = (animationTime + 1) % 1700;
}


