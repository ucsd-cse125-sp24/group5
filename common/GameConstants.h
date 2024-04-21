// Number of entities that can move by itself (4 players for now)
#define NUM_PLAYER_ENTITIES 4
#define NUM_MOVEMENT_ENTITIES NUM_PLAYER_ENTITIES + 0


// Movement parameters
#ifdef __APPLE__
#define MOVEMENT_SPEED 0.15f
#else
#define MOVEMENT_SPEED 0.15f   // windows keys seemed too sensitive
#endif
#define JUMP_SPEED 0.1f
#define GRAVITY 0.003f
#define MAX_JUMPS_ALLOWED 2    // double jump by default
#define GROUND_FRICTION 0.4f
#define AIR_FRICTION 0.6f
#define FASTFALL_INCREASE 2