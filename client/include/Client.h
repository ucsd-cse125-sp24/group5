// client.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include "sge/ShittyGraphicsEngine.h"
#include "ClientGame.h"
#include "sge/UIEntity.h"

void clientLoop(void);
void sleep(int ms);
void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_callback(GLFWwindow* window,  double xpos, double ypos);
void clientLoop(void);

static std::unique_ptr<sge::ParticleEmitterEntity> makeProjParticleEmitterEntity(std::vector<float> colorProbs,
    std::vector<glm::vec4> initColors, std::vector<glm::vec4> endColors, size_t positionIndex);

extern double lastX, lastY;
extern bool enableInput;
extern std::unique_ptr<ClientGame> clientGame;
// C++ inheritance only works when referencing objects as pointers/references
extern std::vector<std::shared_ptr<sge::ModelEntityState>> entities;
extern std::vector<std::shared_ptr<sge::DynamicModelEntityState>> movementEntities;
