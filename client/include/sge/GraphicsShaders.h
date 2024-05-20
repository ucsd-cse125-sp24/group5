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

    /**
     * Shader class for all shaders that require model poses
     */
    class EntityShader : public ShaderProgram {
    public:
        friend class Material;
        EntityShader() = default;
        virtual void initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) override;
        void updateBoneTransforms(std::vector<glm::mat4> &transforms);
        void setAnimated(bool animated) const;
        void updatePerspectiveMat(const glm::mat4 &mat) const;
        void updateViewMat(const glm::mat4 &mat) const;
        void updateModelMat(const glm::mat4 &mat) const;
    protected:
        GLuint perspectivePos; // Uniform position of current perspective matrix within GLSL
        GLuint viewPos; // Uniform position of current view matrix
        GLuint modelPos; // Uniform position of current modelview matrix within GLSL
        GLuint isAnimated;
        GLuint boneTransformPos;
    };

    /**
     * Toon shader class
     */
    class ToonShader : public EntityShader {
    public:
        friend class Material;
        virtual void initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) override;
        void updateCamPos(const glm::vec3 &pos) const;
        void updateLightPerspectiveMat(const glm::mat4 &mat) const;
        void updateLightViewMat(const glm::mat4 &mat) const;
        void updateLightDir(const glm::vec4 &dir) const;
        void updateOutline(bool outline) const;
    private:
        GLuint cameraPositionPos; // Uniform position of current camera position in world coordinates
        GLuint lightPerspectivePos;
        GLuint lightViewPos;
        GLuint lightDirPos;

        void setMaterialUniforms();
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

        GLuint drawOutline;

        GLuint shadowMapTexturePos;
    };

    /**
     * Shader class for postprocessor (renders directly to screen)
     */
    class ScreenShader : public ShaderProgram {
    public:
        virtual void initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) override;
    };

    /**
     * Framebuffer class for shadow maps, postprocessing, etc.
     */
    class FrameBuffer {
    public:
        GLuint gBuffer;
        GLuint gColor;
        GLuint gNormal;
        GLuint gStencilDepth;
    };

    /**
     * Shadowmap class for shadow rendering
     */
    class ShadowMap {
    public:
        void initShadowmap();
        void drawToShadowmap() const;
        void updateShadowmap() const;
        void deleteShadowmap();
        FrameBuffer FBO;
    private:

        const int shadowMapWidth = 4096;
        const int shadowMapHeight = 4096;
    };

    /**
     * Postprocessing for handling outline rendering
     * We normally draw everything to a framebuffer, put that framebuffer in a texture
     * then render that texture on a rectangle to the screen.
     */
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

    void initShaders();

    // Standard shading
    extern ToonShader defaultProgram;
    // Post-processing
    extern ScreenShader screenProgram;
    extern Postprocesser postprocessor;
    // Shadows
    extern EntityShader shadowProgram;
    extern ShadowMap shadowprocessor;

    // Extra declarations of window width/height from ShittyGraphicsEngine.cpp
    extern int windowHeight, windowWidth;
}