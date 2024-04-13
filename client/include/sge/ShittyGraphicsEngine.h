//
// Created by benjx on 4/8/2024.
//

#pragma once
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include <iostream>
#include "Client.h"
#include "sge/GraphicsGeometry.h"
#include "sge/GraphicsShaders.h"

namespace sge {
    extern GLFWwindow *window;
    extern int windowHeight, windowWidth;

    void sgeInit();
    void framebufferSizeCallback(GLFWwindow *window, int width, int height);
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
}
