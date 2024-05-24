#pragma once

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <glm/glm.hpp>
#include <vector>
#include <iostream>


#include "stb_image.h"
namespace sge {

    class UIEntity {
    public:
        UIEntity(const char* path);
        UIEntity(const char* path, float xOffset, float yOffset);
        UIEntity(const char* path, float xOffset, float yOffset, float scale);

        GLuint texture;
        float xOffset, yOffset, scale;
        float width, height;

    private:
        void loadImage(const char* path);
        void scaleWidthAndHeightToScreenCoord(int widthInt, int heightInt);
    };

    extern std::vector<std::shared_ptr<UIEntity>> UIs;

};