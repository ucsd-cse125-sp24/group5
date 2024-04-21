#pragma once
#include <memory>
#include <cassert>
#include "ServerNetwork.h"
#include "NetworkData.h"
#include <glm/glm.hpp>
#include "GameConstants.h"
#include "bge/World.h"
#include "bge/Entity.h"

// this is to fix the circular dependency
class ServerNetwork;

class ServerGame
{

public:

    ServerGame(void);
    ~ServerGame(void);

    void update();
    void handleInitConnection(unsigned int client_id);
    void handleClientActionInput(unsigned int client_id, ClientToServerPacket& packet);

    // Game states of world (e.g. golden egg, season)

    // Game states per client (position, direction, vertical velocity, health, cooldown)
    // in ECS terms: Component[EntityIDs...]
    glm::vec3 positions[NUM_MOVEMENT_ENTITIES];
    glm::vec3 velocities[NUM_MOVEMENT_ENTITIES];
    float yaws[NUM_MOVEMENT_ENTITIES];
    float pitches[NUM_MOVEMENT_ENTITIES];
    
    int doubleJumpUsed[NUM_MOVEMENT_ENTITIES];
    int doubleJumpCD[NUM_MOVEMENT_ENTITIES];    // primary deal with network stream incoming.. 
    bool jumpHeld[NUM_MOVEMENT_ENTITIES];
    // maybe just use this and not the above array? try. todo. 
    

private:
    // IDs for the clients connecting for table in ServerNetwork 
    static unsigned int client_id;

    // The ServerNetwork object
    std::unique_ptr<ServerNetwork> network;

    // data buffer
    char network_data[MAX_PACKET_SIZE];

};