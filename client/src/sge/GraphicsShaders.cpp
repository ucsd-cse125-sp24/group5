//
// Created by benjx on 4/10/2024.
//
#include "sge/GraphicsShaders.h"

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

GLint sge::gBuffer;
GLint sge::gPosition;
GLint sge::gNormal;
GLint sge::gColor;

sge::ShaderProgram sge::defaultProgram;


/**
 * Initialize GLSL shaders
 */
void sge::initShaders()
{
    defaultProgram.initShader("./shaders/static.vert.glsl", "./shaders/toon.frag.glsl");
    defaultProgram.useProgram();
    initDefaultProgram();
}

/**
 * Create a new shader program
 * @param vertexShaderPath Path to vertex shader GLSL file
 * @param fragmentShaderPath Path to fragment shader GLSL file
 */
void sge::ShaderProgram::initShader(std::string vertexShaderPath, std::string fragmentShaderPath) {
    // Load shader source files
    std::string vertexShaderSource = readShaderSource(vertexShaderPath);
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char *vertexShaderSourceC = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSourceC, nullptr);
    glCompileShader(vertexShader);
    GLint compiled;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cout << "Failed to compile vertex shader\n";

        // Print error message when compiling vertex shader
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
        // The maxLength includes the NULL terminator
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &errorLog[0]);
        for (GLchar c : errorLog) {
            std::cout << c;
        }
        exit(EXIT_FAILURE);
    }
    std::string fragmentShaderSource = readShaderSource(fragmentShaderPath);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragmentShaderSourceC = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceC, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cout << "Failed to compile fragment shader\n";

        // Print error message stuff when compiling fragment shader
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);
        // The maxLength includes the NULL terminator
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

    if (!linked) {
        std::cout << "Failed to link shaders\n";
        exit(EXIT_FAILURE);
    }
}

/**
 * Returns a shader file's source code as a string
 * @param filename Path to shader glsl source file
 * @return Shader source code as a string
 */
std::string sge::ShaderProgram::readShaderSource(std::string filename) {
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
 * Tells OpenGL context to use this shader program for renderng stuff
 */
void sge::ShaderProgram::useProgram() {
    glUseProgram(program);
}

/**
 * PRECONDITION: defaultProgram already created, initialize defaultProgram uniform variables
 */
void sge::initDefaultProgram() {
    perspectivePos = glGetUniformLocation(defaultProgram.program, "perspective");
    viewPos = glGetUniformLocation(defaultProgram.program, "view");
    modelPos = glGetUniformLocation(defaultProgram.program, "model");
    cameraPositionPos = glGetUniformLocation(defaultProgram.program, "cameraPosition");

    hasDiffuseMap = glGetUniformLocation(defaultProgram.program, "hasDiffuseMap");
    diffuseTexturePos = glGetUniformLocation(defaultProgram.program, "diffuseTexture");
    glUniform1i(diffuseTexturePos, DIFFUSE_TEXTURE);
    diffuseColor = glGetUniformLocation(defaultProgram.program, "diffuseColor");

    hasSpecularMap = glGetUniformLocation(defaultProgram.program, "hasSpecularMap");
    specularTexturePos = glGetUniformLocation(defaultProgram.program, "specularTexturePos");
    glUniform1i(specularTexturePos, SPECULAR_TEXTURE);
    specularColor = glGetUniformLocation(defaultProgram.program, "specularColor");

    emissiveColor = glGetUniformLocation(defaultProgram.program, "emissiveColor");
    ambientColor = glGetUniformLocation(defaultProgram.program, "ambientColor");

    hasBumpMap = glGetUniformLocation(defaultProgram.program, "hasBumpMap");
    bumpTexturePos = glGetUniformLocation(defaultProgram.program, "bumpTexture");
    glUniform1i(bumpTexturePos, BUMP_MAP);

    hasDisplacementMap = glGetUniformLocation(defaultProgram.program, "hasDisplacementMap");
    displacementTexturePos = glGetUniformLocation(defaultProgram.program, "displacementTexture");
    glUniform1i(displacementTexturePos, DISPLACEMENT_MAP);

    hasRoughMap = glGetUniformLocation(defaultProgram.program, "hasRoughMap");
    roughTexturePos = glGetUniformLocation(defaultProgram.program, "roughTexture");
    roughColor = glGetUniformLocation(defaultProgram.program, "roughColor");
    glUniform1i(roughTexturePos, SHININESS_TEXTURE);
}
