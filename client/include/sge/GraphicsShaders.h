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
#include <cmath>   // for sin() and other math functions
#include <ctime>   // for time()

#include "GameConstants.h"
#include "sge/GraphicsConstants.h"

namespace sge {
    /**
     * Shader program containing vertex, fragment, etc. shaders
     */
    class ShaderProgram {
    public:
        ShaderProgram() = default;
        ~ShaderProgram();
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
        friend class ModelEntityState;
        friend class DynamicModelEntityState;
        virtual void initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) override;
        void updateCamPos(const glm::vec3 &pos) const;
        void updateLightPerspectiveMat(const glm::mat4 &mat) const;
        void updateLightViewMat(const glm::mat4 &mat) const;
        void updateLightDir(const glm::vec4 &dir) const;
        void updateOutline(bool outline) const;
        void updateAltState(int state);
        void updateSeason(Season _season, float blend);
    private:
        GLuint cameraPositionPos; // Uniform position of current camera position in world coordinates
        GLuint lightPerspectivePos;
        GLuint lightViewPos;
        GLuint lightDirPos;

        GLuint alternating;
        GLuint altState;
        GLuint seasons;
        GLuint curSeason;
        GLuint seasonBlend;
        GLuint entityAlternating;
        GLuint entitySeasons;

        void setMaterialUniforms();
        GLuint hasDiffuseMap; // Whether current material has a diffuse map
        GLuint diffuseTexturePos[4];
        GLuint diffuseColor; // GLSL identifier for the array of color vectors

        GLuint hasSpecularMap;
        GLuint specularTexturePos;
        GLuint specularColor;

        GLuint hasShinyMap;
        GLuint shinyTexturePos;
        GLuint shinyColor;

        GLuint drawOutline;

        GLuint shadowMapTexturePos;
    };

    class ParticleShader : public ShaderProgram {
    public:
        virtual void initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath, const std::string &geometryShaderPath);
        void updatePerspectiveMat(const glm::mat4 &mat) const;
        void updateViewMat(const glm::mat4 &mat) const;
        void updateParticleSize(const float size) const;
    private:
        GLuint geometryShader;
        GLuint perspectivePos;
        GLuint viewPos;
        GLuint sizePos;
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
        GLuint gMask;
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
    // Particles
    extern ParticleShader particleProgram;
    // Shadows
    extern EntityShader shadowProgram;
    extern ShadowMap shadowprocessor;

    // Extra declarations of window width/height from ShittyGraphicsEngine.cpp
    extern int windowHeight, windowWidth;


    /*
    Draws 3D line (bullet trail) into the world
    */
    class LineShaderProgram : public ShaderProgram {
    public:
        void initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);
        void updateViewMat(const glm::mat4 &mat);
        void updatePerspectiveMat(const glm::mat4 &mat);
        void renderBulletTrail(const glm::vec3& start, const glm::vec3& end);
        // todo: some method to cleanup VAO VBOs
        void deleteLineShader();
    private:
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;

        GLuint viewPos;
        GLuint perspectivePos;
        GLuint red, green, blue;

        float t = 0.0f;
    };

    /*
    Renders crosshair on screen  (without texture)
    */
    class LineUIShaderProgram : public ShaderProgram {
    public:
        void initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);
        // void drawCrossHair();
        void deleteLineUI();
        void drawCrossHair(float emo);
    private:
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;

        GLint aspectRatioPos;
        GLint scalePos;        // crosshair emotive scale

        const GLfloat a = 0.005f;
        // GLfloat crossHairVertices[8] = {
        //     -0.01f, -0.01f,
        //     0.01f, 0.01f,

        //     0.01f, -0.01f,
        //     -0.01f, 0.01f
        // };

        GLfloat emotiveVertices[12*2] = {
            a,a,    -a,a,   -a,-a,  a,-a,
            3*a,a,  a,3*a,  -a,3*a, -3*a,a,
            -3*a,-a,    -a,-3*a,    a,-3*a, 3*a,-a
        };

        GLuint indices[16] = {
            0,4, 0,5,
            1,6, 1,7,
            2,8, 2,9,
            3,10,3,11
        };
    };

    extern LineShaderProgram lineShaderProgram;
    extern LineUIShaderProgram lineUIShaderProgram;
}