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

    #define SEASON_ICON_DIMENSION 155

    /**
     * Load images for UI into the global vector
    */
    void loadUIs() {
        // Ensure the order matches the enum UIIndex in UIEntity.h
        std::string pathPrefix = (std::string)(PROJECT_PATH) + "/client/assets/";
        std::string filePaths[NUM_UIs] = {
            "SpringIcon.png",
            "SummerIcon.png",   //todo, @Joanne
            "AutumnIcon.png",
            "WinterIcon.png",   //todo, @Joanne

            "vivaldi-logo-transparent.png",
            "rickroll.jpg"

        };

        for (std::string& file : filePaths) {
            UIs.push_back(std::make_shared<UIEntity>(pathPrefix + file));
        }

        // do optional initial settings here (position, scale)
        // UIs[SPRING_ICON]->offset = {14, 28};
        // UIs[AUTUMN_ICON]->offset = {57, 14};

    }

    void renderSeasonIcon(int currentSeason) {

        std::shared_ptr<sge::UIEntity> ui = UIs[currentSeason];

        sge::uiShaderProgram.drawUI(SEASON_ICON_DIMENSION, SEASON_ICON_DIMENSION, 
                                    0, 655, ui->scale, ui->texture);
    }

    void renderGiveUp() {
        std::shared_ptr<sge::UIEntity> ui = UIs[NEVER_GONNA];
        sge::uiShaderProgram.drawUI(SEASON_ICON_DIMENSION, SEASON_ICON_DIMENSION, 
                                    0,0 , 1, ui->texture);
    }

    void renderLogo() {
        std::shared_ptr<sge::UIEntity> ui = UIs[VIVALDI_LOGO];
        sge::uiShaderProgram.drawUI(ui->width, ui->height,
                                    500, 500, 1, ui->texture);
    }

    /**
     * the one for all
    */
    void renderAllUIs(int currentSeason) {
        glEnable(GL_BLEND); // enable alpha blending for images with transparent background
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        sge::renderSeasonIcon(currentSeason);
        sge::renderGiveUp();
        // sge::renderLogo();
        
        glDisable(GL_BLEND);
    }

    // Here, the x,y offset are relative to the window's width (1400) and height (800)
    /**
     * (0,height)           (width, height)
     * 
     * 
     * 
     * (0,0)                (width, 0)
    */

    // the one to render all texts, prolly shouldn't be here but im too lazy to create another text entitiy class
    void renderAllTexts(int myHP, int team1score, int team2score, int season, bool inputEnabled) {
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // sge::textShaderProgram.renderText("Never gonna give you up", 25.0f, 550.0f, 1.3f, glm::vec3(1.0f, 0.8f, 0.2f));
        // sge::textShaderProgram.renderText("Never gonna let you down", 25.0f, 525.0f, 1.3f, glm::vec3(0.8f, 0.8f, 0.2f));
        // sge::textShaderProgram.renderText("Never gonna run around and desert UI", 25.0f, 500.0f, 1.5f, glm::vec3(0.3f, 0.8f, 0.2f));
        
        // Health Points
        std::string hp = std::to_string(myHP);
        glm::vec3 hpColor;
        if (myHP > 65)      hpColor = glm::vec3(0.5, 0.71f, 0.372); // dark green
        else if (myHP > 20) hpColor = glm::vec3(0.9764705882f, 0.6431372549f, 0.01960784314f); // orange
        else                hpColor = glm::vec3(0.99f, 0.02f, 0.01f);  // red
        sge::textShaderProgram.renderText("HP: "+hp, 1220.0f, 730.0f, 1.4f, hpColor);
        
        // Team scores
        std::string score1 = std::to_string(team1score);
        std::string score2 = std::to_string(team2score);
        sge::textShaderProgram.renderText(score1+" - "+score2, 650.0f, 725.0f, 1.4f, glm::vec3(1.0f, 1.0f, 1.0f));
        // todo: use color to signify huge changes to score (e.g. throw egg -> score-=50 to discourage throwing egg --matthew)
        // todo: add an identifer for my team

        // Current Season
        std::vector<std::string> seasons = {"Spring", "Summer", "Autumn", "Winter"};
        std::vector<glm::vec3> seasonColors = {glm::vec3(0.065f, 0.0933f, 0.0565f),
                                           glm::vec3(1.0f, 1.0f, 1.0f),
                                           glm::vec3(0.065f, 0.0933f, 0.0565f),
                                           glm::vec3(1.0f, 1.0f, 1.0f)
                                           };
        sge::textShaderProgram.renderText(seasons[season], 18.0f, 725.0f, 1, seasonColors[season]);
        
        // todo:render time left in game (count down timer in server)

        if (!inputEnabled) {
            sge::textShaderProgram.renderText("-- click here to resume game --", 410, 200, 1, glm::vec3(1.0f, 0.8f, 0.2f));
        }

    
        glDisable(GL_BLEND);
    }

};