// client.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include "sge/ShittyGraphicsEngine.h"
#include "ClientGame.h"

void clientLoop(void);
void sleep(int ms);
void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_callback(GLFWwindow* window,  double xpos, double ypos);
void clientLoop(void);

extern double lastX, lastY;
extern bool enableInput;
extern std::unique_ptr<ClientGame> clientGame;
// C++ inheritance only works when referencing objects as pointers/references
extern std::vector<std::shared_ptr<sge::EntityState>> entities;
extern std::vector<std::shared_ptr<sge::DynamicEntityState>> movementEntities;

