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

enum MovementEntityStateIndex {
    MOVING_HORIZONTALLY = 0,
    ON_GROUND = 1,
    IS_SHOOTING = 2,
    IS_USING_ABILITY = 3,
    NUM_STATES
};

// Number of entities that can move by itself (4 players + 1 egg for now)
#define NUM_PLAYER_ENTITIES 4
#define NUM_EACH_PROJECTILE 4
#define NUM_MOVEMENT_ENTITIES NUM_PLAYER_ENTITIES + NUM_PROJ_TYPES * NUM_EACH_PROJECTILE + 1

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
#define MAX_LAUNCH_SEVERITY 4.0f
#define MAX_HEAL_STRENGTH 20

// Camera parameters
#define CAMERA_DISTANCE_BEHIND_PLAYER std::stof(SetupParser::getValue("camera_distance_behind_player"))
#define CAMERA_DISTANCE_ABOVE_PLAYER 1.35f
