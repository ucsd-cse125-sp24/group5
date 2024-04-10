//
// Created by benjx on 4/10/2024.
//

#ifndef GROUP5_GRAPHICSSHADERS_H
#define GROUP5_GRAPHICSSHADERS_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>

namespace sge {

extern GLuint vertexShader;
extern GLuint fragmentShader;
std::string readShaderSource(std::string filename);
void initShaders();

}
#endif //GROUP5_GRAPHICSSHADERS_H
