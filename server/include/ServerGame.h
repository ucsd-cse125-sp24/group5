#pragma once
#include <memory>
#include <cassert>
#include "ServerNetwork.h"
#include "NetworkData.h"

// this is to fix the circular dependency
class ServerNetwork;

class ServerGame
{

public:

    ServerGame(void);
    ~ServerGame(void);

    void update();

    void reportAllCounters();

    void handleInitConnection(unsigned int client_id);
    void handleIncreaseCounter(unsigned int client_id, IncreaseCounterUpdate increase_counter_update);
    void handleReplaceCounter(unsigned int client_id, ReplaceCounterUpdate replace_counter_update);

private:
    // IDs for the clients connecting for table in ServerNetwork 
    static unsigned int client_id;

    // The ServerNetwork object
    std::unique_ptr<ServerNetwork> network;

    std::map<unsigned int, int> counters;
};