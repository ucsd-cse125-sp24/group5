#include "sge/UIEntity.h"

namespace sge {

    UIEntity::UIEntity(const char* path)
        : UIEntity(path, 0.0f, 0.0f, 1.0f) { 
        // default offset = none; default scale = 1. 
    }

    UIEntity::UIEntity(const char* path, float xOffset, float yOffset) 
        : UIEntity(path, xOffset, yOffset, 1.0f) { 
        // default scale = 1.
    }

    UIEntity::UIEntity(const char* path, float xOffset, float yOffset, float scale) {
        loadImage(path);

        this->xOffset = xOffset;
        this->yOffset = yOffset;
        this->scale = scale;
    }


    void UIEntity::loadImage(const char* path) {
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        // set the texture wrapping/filtering options 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // load and generate the texture
        stbi_set_flip_vertically_on_load(true);  
        int widthInt, heightInt, nrChannels;
        unsigned char *data = stbi_load(path, &widthInt, &heightInt, &nrChannels, 0);
        if (!data) {
            std::printf("Failed to load UI texture from %s\n", path);
            stbi_image_free(data);
            return;
        }
        // Handle different number of channels in texture
        int format = GL_RGB;
        if (nrChannels == 1) {
            format = GL_RED;
        } else if (nrChannels == 4) {
            format = GL_RGBA;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, format, widthInt, heightInt, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);

        glBindTexture(GL_TEXTURE_2D, 0);

        // store scale-to-screen width and height floats in this object
        scaleWidthAndHeightToScreenCoord(widthInt, heightInt);
    }

    /**
     * image loaded has large integer width and height (e.g. 512 x 512)
     * but ui shader draws stuff onto smaller screen coords [-1.0, 1.0] x [-1.0, 1.0]
     * thus this helper method shrinks down width and height to fit screen coords
    */
    void UIEntity::scaleWidthAndHeightToScreenCoord(int widthInt, int heightInt) {
        float div = (float) std::max(widthInt, heightInt);
        width = (float)widthInt / div;
        height = (float)heightInt / div;
    }

    // global vector
    std::vector<std::shared_ptr<UIEntity>> UIs;

};