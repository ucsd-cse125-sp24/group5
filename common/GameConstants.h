// Number of entities that can move by itself (4 players for now)
#define NUM_PLAYER_ENTITIES 4
#define NUM_MOVEMENT_ENTITIES NUM_PLAYER_ENTITIES + 0


// Movement parameters
#ifdef __APPLE__
#define MOVEMENT_SPEED 0.4f
#else
#define MOVEMENT_SPEED 0.2f   // windows keys seemed too sensitive
#endif
#define JUMP_SPEED 0.6f
#define GRAVITY 0.02f
#define MAX_JUMPS_ALLOWED 2    // double jump by default
#define DOUBLE_JUMP_COOLDOWN_TICKS 25