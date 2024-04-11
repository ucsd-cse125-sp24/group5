#include "ServerGame.h"

unsigned int ServerGame::client_id;

ServerGame::ServerGame(void)
{
    // id's to assign clients for our table
    client_id = 0;

    // set up the server network to listen
    network = std::make_unique<ServerNetwork>(this);

}

void ServerGame::update()
{
    std::cout << "Running update" << std::endl;
    // get new clients
    if (network->acceptNewClient(client_id))
    {
        std::printf("client %d has been connected to the server\n", client_id);

        client_id++;
    }

    network->receiveFromClients();
    // do game logic here and then do all the sends back

    // update all clients
    reportAllCounters();
    // sendActionPackets();
}

void ServerGame::handleInitConnection(unsigned int client_id) {
    std::cout << "Server received init packet from client " << client_id << std::endl;
    counters[client_id] = 0;

    // This is a new client, so tell it what its id is
    IssueIdentifierUpdate update;
    update.client_id = client_id;
    network->sendIssueIdentifierUpdate(update);
}

void ServerGame::handleIncreaseCounter(unsigned int client_id, IncreaseCounterUpdate increase_counter_update) {
    counters[client_id] += increase_counter_update.add_amount;
}

void ServerGame::handleReplaceCounter(unsigned int client_id, ReplaceCounterUpdate replace_counter_update) {
    counters[client_id] = replace_counter_update.counter_value;
}

void ServerGame::reportAllCounters() {
    // go through all client counters
    std::map<unsigned int, int>::iterator counter_iter;

    for (counter_iter = counters.begin(); counter_iter != counters.end(); counter_iter++) {
        std::cout << "Counter for client " << counter_iter->first << ": " << counter_iter->second << std::endl;

        ReportCounterUpdate update;
        update.counter_value = counter_iter->second;
        update.client_id = counter_iter->first;

        network->sendReportCounterUpdate(update);
    }
}

ServerGame::~ServerGame(void) {

}
