// client.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "ClientGame.h"

class Client {
    public:
        // Keyboard input stuffs
        void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};