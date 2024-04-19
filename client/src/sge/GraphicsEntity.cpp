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
 *
 * Draws the entity on the screen
 */
void sge::EntityState::draw() const {
    // TODO: add support for server-side roll? Maybe add pitch too here
    models[modelIndex]->render(position, yaw);
}

sge::DynamicEntityState::DynamicEntityState(size_t modelIndex, size_t positionIndex) : EntityState(modelIndex), positionIndex(positionIndex) {
}

void sge::DynamicEntityState::draw() const {
    models[modelIndex]->render(clientGame->positions[positionIndex],
                                clientGame->yaws[positionIndex]
                                ); // TODO: add support for server-side roll? Maybe add pitch too here
}



