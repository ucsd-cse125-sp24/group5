#pragma once

enum PlayerType {
    SPRING_PLAYER,
    SUMMER_PLAYER,
    AUTUMN_PLAYER,
    WINTER_PLAYER
};

enum MovementEntityStateIndex {
    MOVING_HORIZONTALLY = 0,
    ON_GROUND = 1,
    NUM_STATES
};

// Number of entities that can move by itself (4 players + 1 egg for now)
#define NUM_PLAYER_ENTITIES 4
#define NUM_MOVEMENT_ENTITIES NUM_PLAYER_ENTITIES + 1


// Movement parameters
#define MOVEMENT_SPEED 0.30f
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

#define PLAYER_HEALTH 100

// Egg 
#define EGG_CHANGE_OWNER_CD 4
#define EGG_X_WIDTH 0.75f
#define EGG_Z_WIDTH 0.75f
#define EGG_Y_HEIGHT 1.0f

// Camera parameters
#define CAMERA_DISTANCE_BEHIND_PLAYER 3.0f
#define CAMERA_DISTANCE_ABOVE_PLAYER 2.0f
