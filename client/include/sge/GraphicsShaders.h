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
extern GLint perspectivePos; // Uniform position of current perspective matrix within GLSL
extern GLint viewPos; // Uniform position of current view matrix
extern GLint modelPos; // Uniform position of current modelview matrix within GLSL
extern GLint hasDiffuseTexture;
extern GLint diffuseColor;

std::string readShaderSource(std::string filename);
void initShaders();

}
