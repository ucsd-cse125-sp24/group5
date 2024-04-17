#include "ServerGame.h"

unsigned int ServerGame::client_id;
bge::World world;

ServerGame::ServerGame(void)
{
    // id's to assign clients for our table
    client_id = 0;

    // set up the server network to listen
    network = std::make_unique<ServerNetwork>(this);

    world.init();

}

void ServerGame::update()
{
    // get new clients
    if (network->acceptNewClient(client_id))
    {
        std::printf("client %d has been connected to the server\n", client_id);

        client_id++;
    }

    network->receiveFromClients();
    // do game logic here and then do all the sends back

    
}

void ServerGame::handleInitConnection(unsigned int client_id) {
    std::cout << "Server received init packet from client " << client_id << std::endl;

    // This is a new client, so tell it what its id is
    IssueIdentifierUpdate update;
    update.client_id = client_id;
    network->sendIssueIdentifierUpdate(update);
}


void ServerGame::handleClientActionInput(unsigned int client_id, ClientToServerPacket& packet)
{
    // for testing now 
    std::printf("client(%d): W(%d) A(%d) S(%d) D(%d) Jump(%d)\n", client_id, packet.requestForward, packet.requestLeftward, packet.requestBackward, packet.requestRightward, packet.requestJump);

    // todo: update server's game state.
    // (graphics - I also need the camera angle thingy to move character in the right direction).


}

ServerGame::~ServerGame(void) {

}
