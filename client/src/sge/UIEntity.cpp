#include "sge/UIEntity.h"

namespace sge {

    UIEntity::UIEntity(const char* path) {
        loadImage(path);
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
        unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
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
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // global vector
    std::vector<std::shared_ptr<UIEntity>> UIs;

};