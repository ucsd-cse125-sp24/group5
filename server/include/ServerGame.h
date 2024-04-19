#pragma once
#include <memory>
#include <cassert>
#include "ServerNetwork.h"
#include "NetworkData.h"
#include <glm/glm.hpp>
#include "GameConstants.h"

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
    
    int remainingJumps[NUM_MOVEMENT_ENTITIES];


private:
    // IDs for the clients connecting for table in ServerNetwork 
    static unsigned int client_id;

    // The ServerNetwork object
    std::unique_ptr<ServerNetwork> network;

    // data buffer
    char network_data[MAX_PACKET_SIZE];

};