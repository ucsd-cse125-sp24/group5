//
// Created by benjx on 4/10/2024.
//
#include "sge/GraphicsShaders.h"

GLuint sge::vertexShader;
GLuint sge::fragmentShader;
GLuint sge::program;
GLint sge::modelViewPos;

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
    std::string vertexShaderSource = readShaderSource("./shaders/static.vert.glsl");
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char *vertexShaderSourceC = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSourceC, nullptr);
    glCompileShader(vertexShader);
    GLint compiled;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cout << "Failed to compile vertex shader\n";
        exit(EXIT_FAILURE);
    }

    std::string fragmentShaderSource = readShaderSource("./shaders/static.frag.glsl");
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragmentShaderSourceC = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceC, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cout << "Failed to compile fragment shader\n";
        exit(EXIT_FAILURE);
    }

    program = glCreateProgram();
    GLint linked;
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked) {
        glUseProgram(program);
    } else {
        std::cout << "Failed to link shaders\n";
        exit(EXIT_FAILURE);
    }
    modelViewPos = glGetUniformLocation(program, "modelview");

}
