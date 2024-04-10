//
// Created by benjx on 4/8/2024.
//

#ifndef GROUP5_SHITTYGRAPHICSENGINE_H
#define GROUP5_SHITTYGRAPHICSENGINE_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "sge/GraphicsGeometry.h"

namespace sge {
    extern GLFWwindow *window;
    extern int windowHeight, windowWidth;

    void sgeInit();
    void framebufferSizeCallback(GLFWwindow *window, int width, int height);
    void sgeLoop();
}

#endif //GROUP5_SHITTYGRAPHICSENGINE_H
