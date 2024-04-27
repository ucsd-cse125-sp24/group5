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
#include <glm/glm.hpp>
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
    class ShaderProgram; // Forward declaration
    class DefaultShaderProgram;

    extern DefaultShaderProgram defaultProgram;

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
        virtual void initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);

        void useProgram() const;

    protected:
        // Add geometry shader and stuff as needed later
        GLuint vertexShader;
        GLuint fragmentShader;
        // Identifier for shader program
        GLuint program;

        static std::string readShaderSource(const std::string &filename);

        static void printCompileError(GLint shaderID);

        GLint initShader(const std::string &shaderPath, const GLint &shaderType);
    };

    class DefaultShaderProgram : public ShaderProgram {
    public:
        friend class Material;
        DefaultShaderProgram() = default;
        void initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) override;

        void updateCamPos(const glm::vec3 &pos) const;
        void updatePerspectiveMat(const glm::mat4 &mat) const;
        void updateViewMat(const glm::mat4 &mat) const;
        void updateModelMat(const glm::mat4 &mat) const;

    protected:
        GLint perspectivePos; // Uniform position of current perspective matrix within GLSL
        GLint viewPos; // Uniform position of current view matrix
        GLint modelPos; // Uniform position of current modelview matrix within GLSL
        GLint cameraPositionPos; // Uniform position of current camera position in world coordinates

        GLint hasDiffuseMap; // Whether current material has a diffuse map
        GLint diffuseTexturePos;
        GLint diffuseColor;

        GLint hasSpecularMap;
        GLint specularTexturePos;
        GLint specularColor;

        GLint hasBumpMap;
        GLint bumpTexturePos;

        GLint hasDisplacementMap;
        GLint displacementTexturePos;

        GLint hasRoughMap;
        GLint roughTexturePos;
        GLint roughColor;

        GLint emissiveColor;

        GLint ambientColor;
    };

    void initShaders();


}