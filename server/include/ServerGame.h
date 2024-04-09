#pragma once
#include <memory>
#include "ServerNetwork.h"
#include "NetworkData.h"

class ServerGame
{

public:

    ServerGame(void);
    ~ServerGame(void);

    void update();
    void receiveFromClients();
    void sendActionPackets();
    void sendCounterPackets();

private:

    // IDs for the clients connecting for table in ServerNetwork 
    static unsigned int client_id;

    // The ServerNetwork object
    std::unique_ptr<ServerNetwork> network;

    std::map<unsigned int, int> counters;

    // data buffer
    char network_data[MAX_PACKET_SIZE];
};