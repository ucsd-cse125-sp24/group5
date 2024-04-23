// Number of entities that can move by itself (4 players for now)
#define NUM_PLAYER_ENTITIES 4
#define NUM_MOVEMENT_ENTITIES NUM_PLAYER_ENTITIES + 0


// Movement parameters
#define MOVEMENT_SPEED 0.45f
#define AIR_MOVEMENT_MODIFIER 0.5f
#define JUMP_SPEED 0.5f
#define GRAVITY 0.05f
#define MAX_JUMPS_ALLOWED 2    // double jump by default
#define GROUND_FRICTION 0.4f
#define AIR_FRICTION 0.6f
#define FASTFALL_INCREASE 2