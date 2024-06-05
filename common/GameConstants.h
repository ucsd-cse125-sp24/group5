#pragma once
#include <string>
#include "SetupParser.h"

enum PlayerType {
    SPRING_PLAYER,
    SUMMER_PLAYER,
    AUTUMN_PLAYER,
    WINTER_PLAYER
};

enum BallProjType {
    SPRING,
    SUMMER,
    AUTUMN,
    WINTER,
    NUM_PROJ_TYPES
};

enum Season {
    SPRING_SEASON,
    SUMMER_SEASON,
    AUTUMN_SEASON,
    WINTER_SEASON,
    NUM_SEASONS
};

enum MovementEntityStateIndex {
    MOVING_HORIZONTALLY = 0,
    ON_GROUND = 1,
    IS_SHOOTING = 2,
    IS_USING_ABILITY = 3,
    EXPLODING = 4,
    IS_DANCING,
    NUM_STATES
};

enum Teams {
    BLUE = 0,
    RED = 1
};

// Number of entities that can move by itself (4 players + 1 egg for now)
#define NUM_PLAYER_ENTITIES 4
#define NUM_EACH_PROJECTILE 4
#define NUM_TOTAL_PROJECTILES NUM_PROJ_TYPES * NUM_EACH_PROJECTILE 
#define NUM_MOVEMENT_ENTITIES NUM_PLAYER_ENTITIES + NUM_TOTAL_PROJECTILES + 1

// Map
#define HEIGHT_LIMIT 20 // how far above the highest point does the map extend

// Movement parameters
#define MOVEMENT_SPEED 0.30f
#define SLOW_MOVEMENT_SPEED 0.0f
#define AIR_MOVEMENT_MODIFIER 0.5f
#define JUMP_SPEED 0.5f
#define GRAVITY 0.05f
#define MAX_JUMPS_ALLOWED 2    // double jump by default
#define GROUND_FRICTION 0.4f
#define AIR_FRICTION 0.6f
#define FASTFALL_INCREASE 2

// Player
#define PLAYER_X_WIDTH 0.5f
#define PLAYER_Z_WIDTH 0.5f
#define PLAYER_Y_HEIGHT 1.42f

#define SHOOTING_CD_TICKS 3
#define BULLET_MAX_T 70.0f
#define BULLET_FRAMES 5

#define PLAYER_MAX_HEALTH 100

// Egg 
#define EGG_CHANGE_OWNER_CD 4
#define EGG_X_WIDTH 0.75f
#define EGG_Z_WIDTH 0.75f
#define EGG_Y_HEIGHT 1.0f

// Seasonal abilities
#define SEASON_ABILITY_CD 80
#define PROJ_X_WIDTH 0.4f
#define PROJ_Z_WIDTH 0.4f
#define PROJ_Y_HEIGHT 0.4f
#define PROJ_SPEED 0.4f
#define PROJ_MAX_T 70.0f
#define PROJ_EXPLOSION_RADIUS 5.0f
#define PROJ_EXPLOSION_RADIUS_MAX_EFFECT 0.5f
#define MAX_PROJ_EFFECT_LENGTH 160
#define MAX_LAUNCH_SEVERITY 3.0f
#define MAX_HEAL_STRENGTH 20

// Camera parameters
#define CAMERA_DISTANCE_BEHIND_PLAYER std::stof(SetupParser::getValue("camera_distance_behind_player"))
#define CAMERA_DISTANCE_ABOVE_PLAYER 1.35f


// Character UID
#define SPRING_CHARACTER 0
#define SUMMER_CHARACTER 1
#define FALL_CHARACTER 2
#define WINTER_CHARACTER 3
#define NO_CHARACTER INT_MIN
// Lerping
#define LERP_DURATION_TICKS 4.0f

// Dance bomb
#define DANCE_BOMB_DENOTATION_TICKS 20
#define DANCE_BOMB_DURATION_SECS 6
#define DANCE_BOMB_RADIUS 5

// End game condition
#define WINNER_1_POS glm::vec3(std::stof(SetupParser::getValue("win1x")), std::stof(SetupParser::getValue("win1y")), std::stof(SetupParser::getValue("win1z")))
#define WINNER_2_POS glm::vec3(std::stof(SetupParser::getValue("win2x")), std::stof(SetupParser::getValue("win2y")), std::stof(SetupParser::getValue("win2z")))
#define LOSER_1_POS glm::vec3(std::stof(SetupParser::getValue("los1x")), std::stof(SetupParser::getValue("los1y")), std::stof(SetupParser::getValue("los1z")))
#define LOSER_2_POS glm::vec3(std::stof(SetupParser::getValue("los2x")), std::stof(SetupParser::getValue("los2y")), std::stof(SetupParser::getValue("los2z")))
#define GAME_END_CAMERA_DIR glm::vec3(std::stof(SetupParser::getValue("cam2x")), std::stof(SetupParser::getValue("cam2y")), std::stof(SetupParser::getValue("cam2z")))
#define GAME_END_CAMERA_POS glm::vec3(std::stof(SetupParser::getValue("campos2x")), std::stof(SetupParser::getValue("campos2y")), std::stof(SetupParser::getValue("campos2z")))

#define SEASON_LENGTH 500

// Start game 
#define MIN_PLAYERS std::stoi(SetupParser::getValue("min_players_to_start"))
#define GAME_DURATION std::stoi(SetupParser::getValue("game_duration_seconds"))

