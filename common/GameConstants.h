#pragma once

enum PlayerType {
    SPRING_PLAYER,
    SUMMER_PLAYER,
    AUTUMN_PLAYER,
    WINTER_PLAYER
};

enum BallProjType {
    SPRING,
    SUMMER,
    FALL,
    WINTER,
    NUM_PROJ_TYPES
};

enum MovementEntityStateIndex {
    MOVING_HORIZONTALLY = 0,
    ON_GROUND = 1,
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
#define PLAYER_Y_HEIGHT 1.53f

// Egg 
#define EGG_CHANGE_OWNER_CD 4
#define EGG_X_WIDTH 0.75f
#define EGG_Z_WIDTH 0.75f
#define EGG_Y_HEIGHT 1.0f

// Seasonal abilities
#define SEASON_ABILITY_CD 80
#define PROJ_X_WIDTH 0.75f
#define PROJ_Z_WIDTH 0.75f
#define PROJ_Y_HEIGHT 1.00f
#define PROJ_SPEED 0.2f
#define PROJ_EXPLOSION_RADIUS 5.0f
#define MAX_PROJ_EFFECT_LENGTH 160

// Camera parameters
#define CAMERA_DISTANCE_BEHIND_PLAYER 3.0f
#define CAMERA_DISTANCE_ABOVE_PLAYER 2.0f
