#include "ClientGame.h"


ClientGame::ClientGame()
{
    network = std::make_unique<ClientNetwork>();
	// send init packet
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.packet_type = INIT_CONNECTION;

	packet.serialize(packet_data);

	NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);

    counter_start = 0;
    sendCounterReplace(counter_start);
    counter_start++;
}

void ClientGame::sendActionPackets()
{
	// send action packet
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
    packet.packet_type = ACTION_EVENT;

	packet.serialize(packet_data);

	NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
}

void ClientGame::sendCounterIncrease()
{
    // send action packet
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

    Packet packet;
    packet.packet_type = INCREASE_COUNTER;
    IncreaseCounterPacketContents packet_contents;
    packet_contents.add_amount = 5;
    serialize(&packet_contents, packet.contents_data);

    packet.serialize(packet_data);

    NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
}

void ClientGame::sendCounterReplace(int new_value)
{
    // send action packet
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

    Packet packet;
    packet.packet_type = REPLACE_COUNTER;
    ReplaceCounterPacketContents packet_contents;
    packet_contents.counter_value = new_value;
    serialize(&packet_contents, packet.contents_data);

    packet.serialize(packet_data);

    NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
}


void ClientGame::update()
{
    Packet packet;
    int data_length = network->receivePackets(network_data);

    sendCounterIncrease();

    if (data_length <= 0)
    {
        //no data recieved
        return;
    }

    unsigned int i = 0;
    while (i < data_length)
    {
        packet.deserialize(&(network_data[i]));
        i += sizeof(Packet);

        switch (packet.packet_type) {

        case ACTION_EVENT:
            std::printf("client received action event packet from server\n");
            sendActionPackets();
            break;

        case REPORT_COUNTER:
            ReportCounterPacketContents packet_contents;
            deserialize(&packet_contents, packet.contents_data);
            std::printf("counter is now %d\n", packet_contents.counter_value);
            if (packet_contents.counter_value >= 50) {
                sendCounterReplace(counter_start);
                counter_start++;
            }
            break;

        default:
            std::printf("error in packet types\n");
            break;
        }
    }
}

ClientGame::~ClientGame(void) {

}
