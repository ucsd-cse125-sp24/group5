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

#include "sge/GraphicsConstants.h"

namespace sge {
    // Extra declarations of window width/height from ShittyGraphicsEngine.cpp
    extern int windowHeight, windowWidth;

    /**
     * Shader program containing vertex, fragment, etc. shaders
     */
    class ShaderProgram {
    public:
        ShaderProgram() = default;

        // Add more constructors to add support for more shaders (e.g. geometry shader)
        virtual void initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);

        void useShader() const;

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
        virtual void initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) override;
        void updateBoneTransforms(std::vector<glm::mat4> &transforms);
        void setAnimated(bool animated) const;
        void updateCamPos(const glm::vec3 &pos) const;
        void updatePerspectiveMat(const glm::mat4 &mat) const;
        void updateViewMat(const glm::mat4 &mat) const;
        void updateModelMat(const glm::mat4 &mat) const;
    protected:
        GLuint perspectivePos; // Uniform position of current perspective matrix within GLSL
        GLuint viewPos; // Uniform position of current view matrix
        GLuint modelPos; // Uniform position of current modelview matrix within GLSL
        GLuint cameraPositionPos; // Uniform position of current camera position in world coordinates

        GLuint hasDiffuseMap; // Whether current material has a diffuse map
        GLuint diffuseTexturePos;
        GLuint diffuseColor;

        GLuint hasSpecularMap;
        GLuint specularTexturePos;
        GLuint specularColor;

        GLuint hasBumpMap;
        GLuint bumpTexturePos;

        GLuint hasDisplacementMap;
        GLuint displacementTexturePos;

        GLuint hasRoughMap;
        GLuint roughTexturePos;
        GLuint roughColor;

        GLuint emissiveColor;

        GLuint ambientColor;

        GLuint isAnimated;
        GLuint boneTransformPos;
    };

    class LineShaderProgram : public ShaderProgram {
    public:
        void initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);
        void updateViewMat(const glm::mat4 &mat);
        void updatePerspectiveMat(const glm::mat4 &mat);
        void renderBulletTrail(glm::vec3& start, glm::vec3& end);
    private:
        GLuint VAO;
        GLuint VBO;
        
        GLuint viewPos;
        GLuint perspectivePos;

        // some data structure to handle fading bullet 
    };

    class ScreenShader : public ShaderProgram {
    public:
        void initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);
        void updateCamPos(const glm::vec3 &pos) const;
    private:
        GLuint cameraPositionPos;
    };


    void initShaders();

    class FrameBuffer {
    public:
        GLuint gBuffer;
        GLuint gColor;
        GLuint gNormal;
        GLuint gDepth;
    };

    class Postprocesser {
    public:
        void initPostprocessor();
        void resizeFBO() const;
        void deletePostprocessor();
        void drawToFramebuffer() const;
        void drawToScreen() const;
    private:
        FrameBuffer FBO;
        GLuint VAO; // VAO for rendering quad to screen
        GLuint VBO; // VBO for rendering quad to screen
    };

    extern DefaultShaderProgram defaultProgram;
    extern LineShaderProgram lineShaderProgram;
    extern ScreenShader screenProgram;
    extern Postprocesser postprocessor;
}