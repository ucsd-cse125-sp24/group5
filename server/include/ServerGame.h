#pragma once
#include <memory>
#include <cassert>
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
    void reportAllCounters();
    void sendIssueIdentifierUpdate(IssueIdentifierUpdate issue_identifier_update);
    void sendReportCounterUpdate(ReportCounterUpdate report_counter_update);

private:
    void handleInitConnection(unsigned int client_id);
    void handleIncreaseCounter(unsigned int client_id, IncreaseCounterUpdate increase_counter_update);
    void handleReplaceCounter(unsigned int client_id, ReplaceCounterUpdate replace_counter_update);

    // IDs for the clients connecting for table in ServerNetwork 
    static unsigned int client_id;

    // The ServerNetwork object
    std::unique_ptr<ServerNetwork> network;

    std::map<unsigned int, int> counters;

    // data buffer
    char network_data[MAX_PACKET_SIZE];
};