#include "ServerGame.h"

unsigned int ServerGame::client_id;

ServerGame::ServerGame(void)
{
    // id's to assign clients for our table
    client_id = 0;

    // set up the server network to listen
    network = std::make_unique<ServerNetwork>(this);

    // Initialize game world
    std::cout << "Initializing server game world...\n";
    world.init();
    world.printDebug();
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

    // game logic
    world.updateAllSystems();

    // send info to clients (this is called once per tick)
    ServerToClientPacket packet;
    world.fillInGameData(packet);
    network->sendPositionsUpdates(packet);

    BulletPacket bulletPacket;
    world.fillInBulletData(bulletPacket);
    if (bulletPacket.count > 0) {
        network->sendBulletsUpdate(bulletPacket);
    }

    // TODO: send to client all players' characters selection
    LobbyServerToClientPacket characterSelectionPacket;
    world.fillInCharacterSelectionData(characterSelectionPacket);
    network->sendCharacterSelectionUpdate(characterSelectionPacket);
    

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
    // pass information about view direction and movement requests from the client's packet to the world
    // the systems will use whatever the most recent info was before each game tick
    world.updatePlayerInput(client_id, packet.pitch, packet.yaw, packet.requestForward, packet.requestBackward, packet.requestLeftward, packet.requestRightward, packet.requestJump, packet.requestThrowEgg, packet.requestShoot, packet.requestAbility, packet.requestReset);
}

void ServerGame::handleClientLobbyInput(unsigned int client_id, LobbyClientToServerPacket& packet) {
    // in the world, update the player character selection
    world.updatePlayerCharacterSelection(client_id, packet.browsingCharacterUID, packet.characterUID);
}

ServerGame::~ServerGame(void) {

}
