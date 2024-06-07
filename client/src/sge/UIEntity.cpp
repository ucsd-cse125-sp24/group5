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

    #define SEASON_ICON_DIMENSION 0.35f

    /**
     * Load images for UI into the global vector
    */
    void loadUIs() {
        // Ensure the order matches the enum UIIndex in UIEntity.h
        std::string pathPrefix = (std::string)(PROJECT_PATH) + "/client/assets/";
        std::string filePaths[NUM_UIs] = {
            "SpringIcon.png",
            "SummerIcon.png",   
            "AutumnIcon.png",
            "WinterIcon.png",

            "Player1.png",
            "Player2.png",
            "Player3.png",
            "Player4.png",
            "Egg.png",
            "Dancebomb2.png",

            "season-abilities/Ability_sp.PNG",
            "season-abilities/Ability_sp2.PNG",

            "season-abilities/Ability_su.PNG",
            "season-abilities/Ability_su2.PNG",

            "season-abilities/Ability_a.PNG",
            "season-abilities/Ability_a2.PNG",

            "season-abilities/Ability_w.PNG",
            "season-abilities/Ability_w2.PNG",

            "vivaldi-logo-transparent.png",
            "rickroll.jpg"

        };

        for (std::string& file : filePaths) {
            UIs.push_back(std::make_shared<UIEntity>(pathPrefix + file));
        }

        // do optional initial settings here (position, scale)
        // UIs[SPRING_ICON]->offset = {-1.0f, 0.65f};
        // UIs[AUTUMN_ICON]->offset = {-1.0f, 0.65f};

        stbi_set_flip_vertically_on_load(false);  // don't cause trouble for other loadings (models, fonts, etc.)
    }

    void renderSeasonIcon(int currentSeason) {

        std::shared_ptr<sge::UIEntity> ui = UIs[currentSeason];

        sge::uiShaderProgram.drawUI(SEASON_ICON_DIMENSION, SEASON_ICON_DIMENSION, 
                                    -1.0f, 0.65f, ui->scale, ui->texture);

    }

    void renderMyPlayerTag(int my_client_id) {
        std::shared_ptr<sge::UIEntity> ui = UIs[PLAYER_1 + my_client_id];
        sge::uiShaderProgram.drawUI(SEASON_ICON_DIMENSION, SEASON_ICON_DIMENSION, 
                                    -1.0f, -0.8, ui->scale, ui->texture);
    }

    void renderEggTagUI(int client_id, int eggHolderId, bool eggIsDanceBomb) {
        // only render egg/bomb UI if you are the one holding it
        if (eggHolderId == client_id) {
            std::shared_ptr<sge::UIEntity> ui;
            float scale;
            if (!eggIsDanceBomb) {
                ui = UIs[EGG_TAG];
                scale = 1.0f;
            }
            else {
                ui = UIs[DANCE_BOMB_TAG];
                scale = 1.4f;
            }
            sge::uiShaderProgram.drawUI(SEASON_ICON_DIMENSION, SEASON_ICON_DIMENSION,
                                        -0.8, -0.8, scale, ui->texture);
        }
    }

    void renderSeasonAbility(int abilityType, bool waitingCD) {  // my character type, my ability CD
        
        std::shared_ptr<sge::UIEntity> ui = UIs[SPRING_ABILITY + abilityType*2];
        sge::uiShaderProgram.drawUI(SEASON_ICON_DIMENSION, SEASON_ICON_DIMENSION,
                                    0.8, -0.8, 1, ui->texture);
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
    void renderAllUIs(int currentSeason, int my_client_id, int client_id, int eggHolderId, bool eggIsDanceBomb, int abilityType, bool waitingCD) {
        glEnable(GL_BLEND); // enable alpha blending for images with transparent background
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        sge::renderSeasonIcon(currentSeason);
        // sge::renderGiveUp();
        // sge::renderLogo();
        sge::renderMyPlayerTag(my_client_id);

        sge::renderEggTagUI(client_id, eggHolderId, eggIsDanceBomb);

        sge::renderSeasonAbility(abilityType, waitingCD);

        glDisable(GL_BLEND);
    }


    // the one to render all texts, prolly shouldn't be here but im too lazy to create another text entitiy class
    void renderAllTexts(int myHP, int team1score, int team2score, int season, bool inputEnabled, bool gameOver, int winner, double gameDurationInSeconds, int detonationMiliSecs, bool imBombOwner) {
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // x,y offset here are relative to the ortho projection matrix in TextShaderProgram: width (1400) and height (800). 
        // Changes to sge::windowWidth or windowHeight won't affect this part. So DON"T CHANGE NUMBERS HERE.
        /**
         * (0,800)              (1400, 800)
         * 
         * 
         * 
         * (0,0)                (1400, 0)
        */
        // sge::textShaderProgram.renderText("Never gonna give you up", 25.0f, 550.0f, 1.3f, glm::vec3(1.0f, 0.8f, 0.2f));
        // sge::textShaderProgram.renderText("Never gonna let you down", 25.0f, 525.0f, 1.3f, glm::vec3(0.8f, 0.8f, 0.2f));
        // sge::textShaderProgram.renderText("Never gonna run around and desert UI", 0.5f, 0.5f, 1, glm::vec3(0.3f, 0.8f, 0.2f)); // test: this should be in the bottom of screen
        
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
        sge::textShaderProgram.renderText(" - ", 685.0f, 725.0f, 1.4f, glm::vec3(1.0f, 1.0f, 1.0f));
        sge::textShaderProgram.renderText(score1,  600.0f, 725.0f, 1.4f, glm::vec3(0.0f, 0.0f, 1.0f));
        sge::textShaderProgram.renderText(score2, 790.0f, 725.0f, 1.4f, glm::vec3(1.0f, 0.0f, 0.0f));
        // todo: use color to signify huge changes to score (e.g. throw egg -> score-=50 to discourage throwing egg --matthew)
        // todo: add identifer for my team

        // // Current Season
        // std::vector<std::string> seasons = {"Spring", "Summer", "Autumn", "Winter"};
        // std::vector<glm::vec3> seasonColors = {glm::vec3(0.065f, 0.0933f, 0.0565f),
        //                                    glm::vec3(1.0f, 1.0f, 1.0f),
        //                                    glm::vec3(0.065f, 0.0933f, 0.0565f),
        //                                    glm::vec3(1.0f, 1.0f, 1.0f)
        //                                    };
        // sge::textShaderProgram.renderText(seasons[season], 18.0f, 725.0f, 1, seasonColors[season]);
        
        // render time left in game (count down timer in server)
        int timeLeft = (int) std::max(360.0 - gameDurationInSeconds, 0.0);
        int minutes = timeLeft / 60;
        int seconds = timeLeft % 60;
        std::string minuteStr = std::to_string(minutes);
        std::string secondStr = std::to_string(seconds);
        if (seconds < 10) {
            secondStr = "0" + secondStr;
        }
        // std::cout << "time left in game " <<timeLeftStr << "\n";
        sge::textShaderProgram.renderText(minuteStr + ":" + secondStr, 15, 600, 1, glm::vec3(1));

        // render time left before dancebomb explodes! (if i'm the bomb owner)
        if (imBombOwner) {
            std::string secondsStr = std::to_string(detonationMiliSecs);
            // std::printf("detonation mili scecs = %d\n", detonationMiliSecs);
            seconds = detonationMiliSecs / 1000;
            int miliseconds = detonationMiliSecs % 1000;
            secondStr = std::to_string(seconds);
            std::string miliString = std::to_string(miliseconds);

            glm::vec3 color = (seconds < 2) ? glm::vec3(1.0f, 0.1f, 0.1f) : glm::vec3(1.0f, 0.0f, 1.0f); 
            sge::textShaderProgram.renderText("explodes in " + secondStr+":"+miliString, 100, 95, 0.7, color);
        }

        if (!inputEnabled) {
            // sge::textShaderProgram.renderText("-- click here to resume game --", 410, 200, 1, glm::vec3(1.0f, 0.8f, 0.2f));
        }

        if (gameOver) {
            if (winner == 0) {
                sge::textShaderProgram.renderText("TEAM BLUE WINS", 500, 500, 2, glm::vec3(0.00f, 0.27f, 1.00));
            } else if (winner == 1) {
                sge::textShaderProgram.renderText("TEAM RED WINS", 500, 500, 2, glm::vec3(1.00, 0.00f, 0.00f));
            }
        }
    
        glDisable(GL_BLEND);
    }

    void renderAllBillboardTags(glm::vec3* positions, int client_id, bool eggIsDanceBomb, int eggHolderId) {

        // render tags above other players
        glEnable(GL_BLEND); // enable alpha blending for images with transparent background
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
            // todo: render my ability affected?

            if (i == client_id) continue;
            sge::billboardProgram.renderPlayerTag(positions[i], sge::UIs[PLAYER_1 + i]->texture);
        }
        // doesn't render egg/bomb tag above if you're the one holding it (render it in yout UI instead)
        if (eggHolderId != client_id) {
            if (eggIsDanceBomb) {
                sge::billboardProgram.renderPlayerTag(positions[NUM_PLAYER_ENTITIES] + glm::vec3(0,0.5,0), sge::UIs[DANCE_BOMB_TAG]->texture, 2.2f);
            }
            else {
                sge::billboardProgram.renderPlayerTag(positions[NUM_PLAYER_ENTITIES], sge::UIs[EGG_TAG]->texture, 1.3f);
            }
        }

        glDisable(GL_BLEND);

    }

};