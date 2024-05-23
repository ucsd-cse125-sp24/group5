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

        GLuint texture;
        int width, height, nrChannels;
        float xOffset, yOffset, scale;

        UIEntity(const char* path);

        void loadImage(const char* path);
        void scaleToScreenCoord();
    };

    extern std::vector<std::shared_ptr<UIEntity>> UIs;

};