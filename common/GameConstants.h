#pragma once

enum PlayerType {
    SPRING_PLAYER,
    SUMMER_PLAYER,
    AUTUMN_PLAYER,
    WINTER_PLAYER
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
#define PLAYER_X_WIDTH 1.0f
#define PLAYER_Z_WIDTH 1.0f
#define PLAYER_Y_HEIGHT 1.7f

// Egg 
#define EGG_CHANGE_OWNER_CD 4
#define EGG_X_WIDTH 1.0f
#define EGG_Z_WIDTH 1.0f
#define EGG_Y_HEIGHT 1.5f