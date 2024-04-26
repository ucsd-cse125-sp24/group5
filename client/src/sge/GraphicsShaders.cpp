//
// Created by benjx on 4/10/2024.
//
#include "sge/GraphicsShaders.h"

GLuint sge::vertexShader;
GLuint sge::fragmentShader;
GLuint sge::program;

GLint sge::perspectivePos;
GLint sge::viewPos;
GLint sge::modelPos;
GLint sge::cameraPositionPos;

GLint sge::hasDiffuseMap;
GLint sge::diffuseTexturePos;
GLint sge::diffuseColor;

GLint sge::hasSpecularMap;
GLint sge::specularTexturePos;
GLint sge::specularColor;

GLint sge::bumpTexturePos;
GLint sge::displacementTexturePos;

GLint sge::hasRoughMap;
GLint sge::roughTexturePos;
GLint sge::roughColor;

GLint sge::emissiveColor;
GLint sge::ambientColor;

GLint sge::hasBumpMap;
GLint sge::hasDisplacementMap;
/**
 * Returns a shader file's source code as a string
 * @param filename Path to shader glsl source file
 * @return Shader source code as a string
 */
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

/**
 * Initialize GLSL shaders
 */
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
    std::string fragmentShaderSource = readShaderSource("./shaders/toon.frag.glsl");
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragmentShaderSourceC = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceC, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cout << "Failed to compile fragment shader\n";
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &errorLog[0]);
        for (GLchar c : errorLog) {
            std::cout << c;
        }

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
    perspectivePos = glGetUniformLocation(program, "perspective");
    viewPos = glGetUniformLocation(program, "view");
    modelPos = glGetUniformLocation(program, "model");
    cameraPositionPos = glGetUniformLocation(program, "cameraPosition");

    hasDiffuseMap = glGetUniformLocation(program, "hasDiffuseMap");
    diffuseTexturePos = glGetUniformLocation(program, "diffuseTexture");
    glUniform1i(diffuseTexturePos, DIFFUSE_TEXTURE);
    diffuseColor = glGetUniformLocation(program, "diffuseColor");

    hasSpecularMap = glGetUniformLocation(program, "hasSpecularMap");
    specularTexturePos = glGetUniformLocation(program, "specularTexturePos");
    glUniform1i(specularTexturePos, SPECULAR_TEXTURE);
    specularColor = glGetUniformLocation(program, "specularColor");

    emissiveColor = glGetUniformLocation(program, "emissiveColor");
    ambientColor = glGetUniformLocation(program, "ambientColor");

    hasBumpMap = glGetUniformLocation(program, "hasBumpMap");
    bumpTexturePos = glGetUniformLocation(program, "bumpTexture");
    glUniform1i(bumpTexturePos, BUMP_MAP);

    hasDisplacementMap = glGetUniformLocation(program, "hasDisplacementMap");
    displacementTexturePos = glGetUniformLocation(program, "displacementTexture");
    glUniform1i(displacementTexturePos, DISPLACEMENT_MAP);

    hasRoughMap = glGetUniformLocation(program, "hasRoughMap");
    roughTexturePos = glGetUniformLocation(program, "roughTexture");
    roughColor = glGetUniformLocation(program, "roughColor");
    glUniform1i(roughTexturePos, SHININESS_TEXTURE);


}
