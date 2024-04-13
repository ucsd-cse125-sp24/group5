//
// Created by benjx on 4/10/2024.
//

#ifndef GROUP5_GRAPHICSSHADERS_H
#define GROUP5_GRAPHICSSHADERS_H
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

extern GLuint vertexShader;
extern GLuint fragmentShader;
extern GLuint program;
extern GLint modelViewPos;

std::string readShaderSource(std::string filename);
void initShaders();

}
#endif //GROUP5_GRAPHICSSHADERS_H
