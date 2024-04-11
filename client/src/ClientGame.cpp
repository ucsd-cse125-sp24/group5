#include "ClientGame.h"


ClientGame::ClientGame()
{
    network = std::make_unique<ClientNetwork>();
	// send init packet
	const unsigned int packet_size = sizeof(ClientToServerPacket);
	char packet_data[packet_size];

	ClientToServerPacket packet;
	packet.packet_type = INIT_CONNECTION;

	packet.serialize(packet_data);

	NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
}

void ClientGame::sendActionPackets()
{
	// send action packet
	const unsigned int packet_size = sizeof(ClientToServerPacket);
	char packet_data[packet_size];

	ClientToServerPacket packet;
	packet.packet_type = ACTION_EVENT;

	packet.serialize(packet_data);

	NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
}


void ClientGame::update()
{
    // Receive packet from server (todo: update type to ServerToClientPacket)
    Packet packet;
    int data_length = network->receivePackets(network_data);

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
            // sendActionPackets();  // be quiet for now
            break;

        default:
            std::printf("error in packet types\n");
            break;
        }
    }
}

void ClientGame::sendClientInputToServer()
{
    ClientToServerPacket packet;
    packet.packet_type = ACTION_EVENT;
    unsigned int size = sizeof(ClientToServerPacket);

    // Movement requests
    packet.requestForward = requestForward;
    packet.requestBackward = requestBackward;
    packet.requestLeftward = requestLeftward;
    packet.requestRightward = requestRightward;
    packet.requestJump = requestJump;

    // (todo: other requests, e.g. shooting, skill)

    
    // Serialize and send to server
    char data[size];
    packet.serialize(data);
	NetworkServices::sendMessage(network->ConnectSocket, data, size);
}

ClientGame::~ClientGame(void) {

}
