#include "sge/UIEntity.h"

namespace sge {

    UIEntity::UIEntity(std::string path)
        : UIEntity(path, 0.0f, 0.0f, 1.0f) { 
        // default offset = none; default scale = 1. 
    }

    // UIEntity::UIEntity(std::string path, float xOffset, float yOffset) 
    //     : UIEntity(path, xOffset, yOffset, 1.0f) { 
    //     // default scale = 1.
    // }

    UIEntity::UIEntity(std::string path, float xOffset, float yOffset, float scale) {
        loadImage(path.c_str());

        this->offset = glm::vec2(xOffset, yOffset);
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

//////////////////////////////////////////Global stuffs below ///////////////////////////////////////////////

    // global vector
    std::vector<std::shared_ptr<UIEntity>> UIs;

    #define SEASON_ICON_DIMENSION 0.4f

    /**
     * Load images for UI into the global vector
    */
    void loadUIs() {
        // Ensure the order matches the enum UIIndex in UIEntity.h
        std::string pathPrefix = (std::string)(PROJECT_PATH) + "/client/assets/";
        std::string filePaths[NUM_UIs] = {
            "SpringIcon.png",
            "SummerIcon.png",   //todo
            "AutumnIcon.png",
            "WinterIcon.png",   //todo

            "vivaldi-logo-transparent.png",
            "rickroll.jpg"

        };

        for (std::string& file : filePaths) {
            UIs.push_back(std::make_shared<UIEntity>(pathPrefix + file));
        }

        // do optional initial settings here (position, scale)
        UIs[SPRING_ICON]->offset = {-1.0f, 0.6f};
        UIs[AUTUMN_ICON]->offset = {-0.8f, 0.6f};

    }

    void renderSeasonIcon(int currentSeason) {

        std::shared_ptr<sge::UIEntity> ui = UIs[currentSeason];

        sge::uiShaderProgram.drawUI(SEASON_ICON_DIMENSION, SEASON_ICON_DIMENSION, 
                                    ui->offset.x, ui->offset.y, ui->scale, ui->texture);

    }

    void renderGiveUp() {
        std::shared_ptr<sge::UIEntity> ui = UIs[NEVER_GONNA];
        sge::uiShaderProgram.drawUI(SEASON_ICON_DIMENSION, SEASON_ICON_DIMENSION, 
                                    0.7, -0.8, 1, ui->texture);
    }

    void renderLogo() {
        std::shared_ptr<sge::UIEntity> ui = UIs[VIVALDI_LOGO];
        sge::uiShaderProgram.drawUI(ui->width, ui->height,
                                    -1.0, -1.0, 1, ui->texture);
    }

    /**
     * the one for all
    */
    void renderAllUIs() {
        glEnable(GL_BLEND); // enable alpha blending for images with transparent background
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        sge::renderSeasonIcon(0);
        sge::renderSeasonIcon(2);
        sge::renderGiveUp();
        sge::renderLogo();
        
        glDisable(GL_BLEND);
    }


    // the one to render all texts, prolly shouldn't be here but im too lazy to create another text entitiy class
    void renderAllTexts(int myHP) {
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // x,y offset here are relative to the window's width (1400) and height (800)
        /**
         * (0,height*2)             (width*2, height*2)
         * 
         * 
         * 
         * (0,0)                    (width*2, 0)
        */
        sge::textShaderProgram.renderText("Never gonna give you up", 25.0f, 1100.0f, 1.3f, glm::vec3(1.0f, 0.8f, 0.2f));
        sge::textShaderProgram.renderText("Never gonna let you down", 25.0f, 1050.0f, 1.3f, glm::vec3(0.8f, 0.8f, 0.2f));
        sge::textShaderProgram.renderText("Never gonna run around and desert UI", 25.0f, 1000.0f, 1.5f, glm::vec3(0.3f, 0.8f, 0.2f));
        
        std::string hp = std::to_string(myHP);
        sge::textShaderProgram.renderText("HP: "+hp, 2500.0f, 1450.0f, 2.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        
        
        glDisable(GL_BLEND);
    }

};