#include "ServerGame.h"

unsigned int ServerGame::client_id;

ServerGame::ServerGame(void)
{
    // id's to assign clients for our table
    client_id = 0;

    // set up the server network to listen
    network = std::make_unique<ServerNetwork>();
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
    receiveFromClients();
}

void ServerGame::receiveFromClients()
{
    // go through all clients
    std::map<unsigned int, SOCKET>::iterator iter;

    for (iter = network->sessions.begin(); iter != network->sessions.end(); /* no increment*/) {
        int data_length = network->receiveData(iter->first, network_data);

        if (data_length == -1) {
            // waiting for msg, nonblocking
            iter++;
            continue;
        } else if (data_length == 0) {
            // no data recieved, ending session
            std::cout << "No data received (data_length=" << data_length << "), ending session.\n";
            network->sessions.erase(iter++);  // trick to remove while iterating
            continue;
        }

        unsigned int i = 0;
        while (i < data_length) {
            UpdateHeader update_header;
            deserialize(&update_header, &(network_data[i]));
            unsigned int data_loc = i + sizeof(UpdateHeader);
            unsigned int update_length = update_type_data_lengths.at(update_header.update_type);

            switch (update_header.update_type) {

            case INIT_CONNECTION:
                handleInitConnection(iter->first);
                break;

            case ACTION_EVENT:
                break;

            case INCREASE_COUNTER:
                IncreaseCounterUpdate increase_counter_update;
                deserialize(&increase_counter_update, &(network_data[data_loc]));

                handleIncreaseCounter(iter->first, increase_counter_update);
                break;

            case REPLACE_COUNTER:
                ReplaceCounterUpdate replace_counter_update;
                deserialize(&replace_counter_update, &(network_data[data_loc]));

                handleReplaceCounter(iter->first, replace_counter_update);
                break;

            default:
                std::cout << "Error in packet types" << std::endl;
                // This should never happen, so assert false so we find out if it does
                assert(false);

                break;
            }
            // Move on to the next update
            i += sizeof(UpdateHeader) + update_length;
        }
        iter++;
    }
    // update all clients
    sendCounterPackets();
    // sendActionPackets();
}

void ServerGame::handleInitConnection(unsigned int client_id) {
    std::cout << "Server received init packet from client " << client_id << std::endl;
    counters[client_id] = 0;
}

void ServerGame::handleIncreaseCounter(unsigned int client_id, IncreaseCounterUpdate increase_counter_update) {
    counters[client_id] += increase_counter_update.add_amount;
}

void ServerGame::handleReplaceCounter(unsigned int client_id, ReplaceCounterUpdate replace_counter_update) {
    counters[client_id] = replace_counter_update.counter_value;
}

void ServerGame::sendActionPackets()
{
    // send action packet
    const unsigned int packet_size = sizeof(UpdateHeader);
    char packet_data[packet_size];

    UpdateHeader header;
    header.update_type = ACTION_EVENT;

    serialize(&header, packet_data);

    network->sendToAll(packet_data, packet_size);
}

void ServerGame::sendCounterPackets()
{
    // go through all client counters
    std::map<unsigned int, int>::iterator counter_iter;

    for (counter_iter = counters.begin(); counter_iter != counters.end(); counter_iter++) {
        std::cout << "Counter for client " << counter_iter->first << ": " << counter_iter->second << std::endl;

        // create packet with updated counter
        const unsigned int packet_size = sizeof(UpdateHeader) + sizeof(ReportCounterUpdate);
        char packet_data[packet_size];

        UpdateHeader header;
        header.update_type = REPORT_COUNTER;
        serialize(&header, packet_data);

        ReportCounterUpdate update;
        update.counter_value = counter_iter->second;
        serialize(&update, packet_data + sizeof(UpdateHeader));

        network->sendToClient(counter_iter->first, packet_data, packet_size);
    }
}

ServerGame::~ServerGame(void) {

}
