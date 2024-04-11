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

    ClientToServerPacket packet;

    // go through all clients
    std::map<unsigned int, SOCKET>::iterator iter;

    for (iter = network->sessions.begin(); iter != network->sessions.end(); /* no increment*/)
    {
        int data_length = network->receiveData(iter->first, network_data);

        if (data_length == -1) 
        {
            // waiting for msg, nonblocking
            iter++;
            continue;
        }
        else if (data_length == 0)
        {
            // no data recieved, ending session 
            std::cout << "No data received (data_lenght=" << data_length << "), ending session.\n";
            network->sessions.erase(iter++);  // trick to remove while iterating
            continue;
        }

        // go through each packet received from that client. 
        unsigned int i = 0;
        while (i < data_length)
        {
            packet.deserialize(&(network_data[i]));
            
            i += sizeof(ClientToServerPacket);

            switch (packet.packet_type) {

            case INIT_CONNECTION: {

                std::printf("server received init packet from client\n");

                sendActionPackets();

                break;
            }
            case ACTION_EVENT: {

                std::printf("server received action event packet from client\n");  // be quiet now

                // sendActionPackets();  // Note! Every data recevied from client would cause the server to send_to_all() clients... Fix this by tagging client_id. 
                unsigned int client_id = iter->first;
                handleActionEvent(client_id, packet);
                break;
            }
            default: {
                std::printf("error in packet types\n");

                break;
            }   
            }
        }
        iter++;
    }
}


void ServerGame::handleActionEvent(unsigned int client_id, ClientToServerPacket& packet)
{
    // for testing now 
    std::printf("client(%d): W(%d) A(%d) S(%d) D(%d) Jump(%d)\n", client_id, packet.requestForward, packet.requestLeftward, packet.requestBackward, packet.requestRightward, packet.requestJump);

    // todo: update server's game state.
    // (graphics - I also need the camera angle thingy to move character in the right direction).


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
