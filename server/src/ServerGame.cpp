#include "ServerGame.h"



unsigned int ServerGame::num_clients;

ServerGame::ServerGame(void)
{
    // id's to assign clients for our table
    num_clients = 0;

    // set up the server network to listen
    network = std::make_unique<ServerNetwork>();
}

void ServerGame::update()
{

    // get new clients
    if (network->acceptNewClient(num_clients))
    {
        std::printf("client %d has been connected to the server\n", num_clients);

        num_clients++;
    }
    receiveFromClients();
}

void ServerGame::receiveFromClients()
{

    Packet packet;

    // go through all clients
    for (unsigned int i = 0; i < num_clients; i++)
    {
        int data_length = network->receiveData(i, network_data);

        if (data_length <= 0)
        {
            //no data recieved
            continue;
        }

        unsigned int j = 0;
        while (j < data_length)
        {
            packet.deserialize(&(network_data[i]));
            j += sizeof(Packet);

            switch (packet.packet_type) {

            case INIT_CONNECTION:

                std::printf("server received init packet from client\n");

                sendActionPackets();

                break;

            case ACTION_EVENT:

                std::printf("server received action event packet from client\n");

                sendActionPackets();

                break;

            default:

                std::printf("error in packet types\n");

                break;
            }
        }
    }
}

void ServerGame::sendActionPackets()
{
    // send action packet
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

    Packet packet;
    packet.packet_type = ACTION_EVENT;

    packet.serialize(packet_data);

    network->sendToAll(packet_data, packet_size);
}

ServerGame::~ServerGame(void) {

}
