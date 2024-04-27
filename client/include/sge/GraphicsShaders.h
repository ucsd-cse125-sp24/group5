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
#include <vector>

/**
 * Texture types
 */
enum TexType {
    DIFFUSE_TEXTURE = 0,
    SPECULAR_TEXTURE = 1,
    BUMP_MAP = 2,
    DISPLACEMENT_MAP = 3,
    SHININESS_TEXTURE = 4,
    UNKNOWN_TEXTYPE = 5,
    NUM_TEXTURES = 6
};

namespace sge {
    class ShaderProgram;
    extern ShaderProgram defaultProgram;

    extern GLint perspectivePos; // Uniform position of current perspective matrix within GLSL
    extern GLint viewPos; // Uniform position of current view matrix
    extern GLint modelPos; // Uniform position of current modelview matrix within GLSL
    extern GLint cameraPositionPos; // Uniform position of current camera position in world coordinates

    extern GLint hasDiffuseMap; // Whether current material has a diffuse map
    extern GLint diffuseTexturePos;
    extern GLint diffuseColor;

    extern GLint hasSpecularMap;
    extern GLint specularTexturePos;
    extern GLint specularColor;

    extern GLint hasBumpMap;
    extern GLint bumpTexturePos;

    extern GLint hasDisplacementMap;
    extern GLint displacementTexturePos;

    extern GLint hasRoughMap;
    extern GLint roughTexturePos;
    extern GLint roughColor;

    extern GLint emissiveColor;

    extern GLint ambientColor;

    extern GLint gBuffer;
    extern GLint gPosition;
    extern GLint gNormal;
    extern GLint gColor;

    /**
     * Shader program containing vertex, fragment, etc. shaders
     */
    class ShaderProgram {
    public:
        ShaderProgram() = default;

        // Add more constructors to add support for more shaders (e.g. geometry shader)
        void initShader(std::string vertexShaderPath, std::string fragmentShaderPath);

        void useProgram();

        // Identifier for shader program
        GLuint program; // Public so uniform shader variables can be used
    private:
        // Add geometry shader and stuff as needed later
        GLuint vertexShader;
        GLuint fragmentShader;

        std::string readShaderSource(std::string filename);
    };

    void initShaders();

    // defaultProgram gets a special function due to its uniform variables
    void initDefaultProgram();

}