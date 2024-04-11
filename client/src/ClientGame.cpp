#include "ClientGame.h"

ClientGame::ClientGame()
{
    network = std::make_unique<ClientNetwork>(this);
	// send init packet
    const unsigned int packet_size = sizeof(UpdateHeader);
    char packet_data[packet_size];

    UpdateHeader header;
    header.update_type = INIT_CONNECTION;

    serialize(&header, packet_data);

	NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);

    counter_start = 0;
    ReplaceCounterUpdate update;
    update.counter_value = counter_start;
    network->sendReplaceCounterUpdate(update);
    counter_start++;
}

void ClientGame::update()
{
    IncreaseCounterUpdate update;
    update.add_amount = 5;
    network->sendIncreaseCounterUpdate(update);

    network->receiveUpdates();
}

void ClientGame::handleActionEvent() {
    std::cout << "Client received action event packet from server" << std::endl;
    network->sendActionUpdate();
}

void ClientGame::handleReportCounter(ReportCounterUpdate report_counter_update) {
    if (report_counter_update.client_id == client_id) {
        // This report is for us
        std::cout << "Counter is now " << report_counter_update.counter_value << std::endl;
        if (report_counter_update.counter_value >= 50) {
            ReplaceCounterUpdate update;
            update.counter_value = counter_start;
            network->sendReplaceCounterUpdate(update);
            counter_start++;
        }
    }
}

void ClientGame::handleIssueIdentifier(IssueIdentifierUpdate issue_identifier_update) {
    client_id = issue_identifier_update.client_id;
    std::cout << "My id is " << client_id << std::endl;
}

ClientGame::~ClientGame(void) {

}
