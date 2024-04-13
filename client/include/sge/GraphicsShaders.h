//
// Created by benjx on 4/10/2024.
//

#pragma once

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>

namespace sge {

// TODO: add support for more shaders
extern GLuint vertexShader; // Identifier for vertex shader
extern GLuint fragmentShader; // Identifier for fragment shader
extern GLuint program; // Identifier for shader program
extern GLint modelViewPos; // Uniform position of current modelview matrix within GLSL

std::string readShaderSource(std::string filename);
void initShaders();

}
