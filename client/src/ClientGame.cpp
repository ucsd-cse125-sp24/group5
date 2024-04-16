#include "ClientGame.h"

ClientGame::ClientGame()
{
    network = std::make_unique<ClientNetwork>(this);
    client_id = 0;
    std::cout << "Initializing client game world...\n";
    for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
        positions[i] = glm::vec3(i*5.0, 0.0, 0.0);
    }

	// send init packet
	network->sendInitUpdate();
}

void ClientGame::handleServerActionEvent(ServerToClientPacket& updatePacket) {

    memcpy(&positions, &updatePacket.positions, sizeof(positions));
    
    // todo: Handle action update (change position, camera angle, HP, etc.)
    

    // network->sendActionUpdate(); // client does not need to notify server of its action. 
}

void ClientGame::sendClientInputToServer()
{
    ClientToServerPacket packet;

    // Movement requests
    packet.requestForward = requestForward;
    packet.requestBackward = requestBackward;
    packet.requestLeftward = requestLeftward;
    packet.requestRightward = requestRightward;
    packet.requestJump = requestJump;

    // (todo: other requests, e.g. shooting, skill)

    // Movement angle
    packet.yaw = yaw;
    packet.pitch = pitch;
    

    // Serialize and send to server
	network->sendClientToServerPacket(packet);
}

void ClientGame::handleIssueIdentifier(IssueIdentifierUpdate issue_identifier_update) {
    client_id = issue_identifier_update.client_id;
    std::cout << "My id is " << client_id << std::endl;
}

ClientGame::~ClientGame(void) {

}
