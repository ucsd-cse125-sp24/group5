#pragma once

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>

#include "sge/GraphicsShaders.h"
#include "stb_image.h"

enum UIIndex {
    SPRING_ICON,
    SUMMER_ICON,
    AUTUMN_ICON,
    WINTER_ICON,

    PLAYER_1,
    PLAYER_2,
    PLAYER_3,
    PLAYER_4,
    EGG_TAG,

    VIVALDI_LOGO,
    NEVER_GONNA,

    NUM_UIs
};

namespace sge {

    class UIEntity {
    public:
        UIEntity(std::string path);
        // UIEntity(std::string path, float xOffset, float yOffset);
        UIEntity(std::string path, float xOffset, float yOffset, float scale);

        GLuint texture;
        float scale;  
        glm::vec2 offset;

        float width, height;

    private:
        void loadImage(const char* path);
        void scaleWidthAndHeightToScreenCoord(int widthInt, int heightInt);
    };

    extern std::vector<std::shared_ptr<UIEntity>> UIs;

    void loadUIs();
    void renderSeasonIcon(int currentSeason);
    void renderMyPlayerTag(int my_client_id);
    void renderGiveUp();
    void renderLogo();

    // the one for all
    void renderAllTexts(int myHP, int team1score, int team2score, int currentSeason, bool inputEnabled, bool gameOver, int winner);
    void renderAllUIs(int currentSeason, int my_client_id);

};