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
    for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
        positions[i] = glm::vec3(i*10.0f, 0.0f, -(i%2)*8.0f);
        velocities[i] = glm::vec3(0.0f, 0.0f, 0.0f);
        doubleJumpUsed[i] = 0;
        jumpHeld[i] = false;
    }
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
    // Book keeping for each client (so they can render each other)

    // Update player's moving direction (no pitch here, cuz you can't fly)
    glm::vec3 forward_direction;
    forward_direction.x = cos(glm::radians(packet.yaw));
    forward_direction.y = 0;
    forward_direction.z = sin(glm::radians(packet.yaw));
    forward_direction = glm::normalize(forward_direction);

    world.updatePlayerInput(client_id, forward_direction, packet.pitch, packet.yaw, packet.requestForward, packet.requestBackward, packet.requestLeftward, packet.requestRightward, packet.requestJump);
}

ServerGame::~ServerGame(void) {

}
