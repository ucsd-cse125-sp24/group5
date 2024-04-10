//
// Created by benjx on 4/10/2024.
//
#include "sge/GraphicsShaders.h"

GLuint sge::vertexShader;
GLuint sge::fragmentShader;

std::string sge::readShaderSource(std::string filename) {
    std::ifstream in;
    in.open(filename);
    if (!in.is_open()) {
        std::cout << "Failed to read shader file: " << filename << "\n";
        exit(EXIT_FAILURE);
    }
    std::string currLine;
    std::string out;
    while (in) {
        std::getline(in, currLine);
        out += currLine + "\n";
    }
    return out;
}

void sge::initShaders()
{
    std::string source = readShaderSource("./shaders/static.vert.glsl");
    glCreateShader(GL_VERTEX_SHADER);

    glCreateShader(GL_FRAGMENT_SHADER);
}
