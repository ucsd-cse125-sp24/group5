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
    

private:
    // IDs for the clients connecting for table in ServerNetwork 
    static unsigned int client_id;

    // The ServerNetwork object
    std::unique_ptr<ServerNetwork> network;

    // data buffer
    char network_data[MAX_PACKET_SIZE];

    bge::World world;

};