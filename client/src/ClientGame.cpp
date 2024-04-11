#include "ClientGame.h"

ClientGame::ClientGame()
{
    network = std::make_unique<ClientNetwork>(this);
	// send init packet
	network->sendInitUpdate();

    // ReplaceCounterUpdate update;
    // update.counter_value = counter_start;
    // network->sendReplaceCounterUpdate(update);
    // counter_start++;
}

void ClientGame::handleActionEvent() {
    std::cout << "Client received action event packet from server" << std::endl;
    network->sendActionUpdate();
}

void ClientGame::handleReportCounter(ReportCounterUpdate report_counter_update) {
    if (report_counter_update.client_id == client_id) {
        // This report is for us
        std::cout << "Counter is now " << report_counter_update.counter_value << std::endl;
        /*if (report_counter_update.counter_value >= 50) {
            ReplaceCounterUpdate update;
            update.counter_value = counter_start;
            network->sendReplaceCounterUpdate(update);
            counter_start++;
        }*/
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
