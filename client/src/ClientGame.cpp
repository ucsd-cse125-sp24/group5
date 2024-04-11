#include "ClientGame.h"

ClientGame::ClientGame()
{
    network = std::make_unique<ClientNetwork>(this);
	// send init packet
	network->sendInitUpdate();
}

void ClientGame::handleServerActionEvent() {
    std::cout << "Client received action event packet from server" << std::endl;
    // todo: Handle action update (change position, camera angle, HP, etc.)

    // network->sendActionUpdate(); // client does not need to notify server of its action. 
}

void ClientGame::handleReportCounter(ReportCounterUpdate report_counter_update) {
    if (report_counter_update.client_id == client_id) {
        // This report is for us
        // std::cout << "Counter is now " << report_counter_update.counter_value << std::endl;
    }
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

    // Serialize and send to server
	network->sendClientToServerPacket(packet);
}

void ClientGame::handleIssueIdentifier(IssueIdentifierUpdate issue_identifier_update) {
    client_id = issue_identifier_update.client_id;
    std::cout << "My id is " << client_id << std::endl;
}

ClientGame::~ClientGame(void) {

}
