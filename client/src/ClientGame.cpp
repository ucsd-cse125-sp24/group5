#include "ClientGame.h"


ClientGame::ClientGame()
{
    network = std::make_unique<ClientNetwork>();
	// send init packet
    const unsigned int packet_size = sizeof(UpdateHeader);
    char packet_data[packet_size];

    UpdateHeader header;
    header.update_type = INIT_CONNECTION;

    serialize(&header, packet_data);

	NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);

    counter_start = 0;
    sendCounterReplace(counter_start);
    counter_start++;
}

void ClientGame::sendActionPackets()
{
    // send action packet
    const unsigned int packet_size = sizeof(UpdateHeader);
    char packet_data[packet_size];

    UpdateHeader header;
    header.update_type = ACTION_EVENT;

    serialize(&header, packet_data);

	NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
}

void ClientGame::sendCounterIncrease()
{
    const unsigned int packet_size = sizeof(UpdateHeader) + sizeof(IncreaseCounterUpdate);
    char packet_data[packet_size];

    UpdateHeader header;
    header.update_type = INCREASE_COUNTER;

    serialize(&header, packet_data);

    IncreaseCounterUpdate update;
    update.add_amount = 5;
    serialize(&update, packet_data + sizeof(UpdateHeader));

    NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
}

void ClientGame::sendCounterReplace(int new_value)
{
    const unsigned int packet_size = sizeof(UpdateHeader) + sizeof(IncreaseCounterUpdate);
    char packet_data[packet_size];

    UpdateHeader header;
    header.update_type = REPLACE_COUNTER;

    serialize(&header, packet_data);

    ReplaceCounterUpdate update;
    update.counter_value = new_value;
    serialize(&update, packet_data + sizeof(UpdateHeader));

    NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
}


void ClientGame::update()
{
    sendCounterIncrease();


    int data_length = network->receivePackets(network_data);

    if (data_length <= 0)
    {
        //no data recieved
        return;
    }

    unsigned int i = 0;
    while (i < data_length)
    {
        UpdateHeader update_header;
        deserialize(&update_header, &(network_data[i]));
        unsigned int data_loc = i + sizeof(UpdateHeader);
        unsigned int update_length = update_type_data_lengths.at(update_header.update_type);

        switch (update_header.update_type) {

        case ACTION_EVENT:
            std::printf("client received action event packet from server\n");
            sendActionPackets();
            break;

        case REPORT_COUNTER:
            ReportCounterUpdate report_counter_update;
            deserialize(&report_counter_update, &(network_data[data_loc]));
            std::printf("counter is now %d\n", report_counter_update.counter_value);
            if (report_counter_update.counter_value >= 50) {
                sendCounterReplace(counter_start);
                counter_start++;
            }
            break;

        default:
            std::printf("error in packet types\n");
            break;
        }
        i += sizeof(UpdateHeader) + data_length;
    }
}

ClientGame::~ClientGame(void) {

}
