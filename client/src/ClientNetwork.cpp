#include "ClientNetwork.h"

/*
 * This file contains the main client networking code 
 * Each type of update should have a send function in here
 * They should vaguely follow the format of functions in here
 * (see sendIncreaseCounterUpdate for outline)
 */

void ClientNetwork::sendIncreaseCounterUpdate(IncreaseCounterUpdate increase_counter_update)
{
	// packet size needs to be const to put packet_data on the stack
    const unsigned int packet_size = sizeof(UpdateHeader) + sizeof(IncreaseCounterUpdate);
    char packet_data[packet_size];

	// create and populate header
    UpdateHeader header;
    header.update_type = INCREASE_COUNTER;

	// serialize header and packet data
    serialize(&header, packet_data);
    serialize(&increase_counter_update, packet_data + sizeof(UpdateHeader));

	// send packet
    NetworkServices::sendMessage(ConnectSocket, packet_data, packet_size);
}

void ClientNetwork::sendClientToServerPacket(ClientToServerPacket packet) {
	// packet size needs to be const to put packet_data on the stack
    const unsigned int packet_size = sizeof(UpdateHeader) + sizeof(ClientToServerPacket);
    char packet_data[packet_size];

	// create and populate header
    UpdateHeader header;
    header.update_type = CLIENT_TO_SERVER;

	// serialize header and packet data
    serialize(&header, packet_data);
    serialize(&packet, packet_data + sizeof(UpdateHeader));

	// send packet
    NetworkServices::sendMessage(ConnectSocket, packet_data, packet_size);
}

void ClientNetwork::sendActionUpdate()
{
    // send action packet
    const unsigned int packet_size = sizeof(UpdateHeader);
    char packet_data[packet_size];

    UpdateHeader header;
    header.update_type = ACTION_EVENT;

    serialize(&header, packet_data);

	NetworkServices::sendMessage(ConnectSocket, packet_data, packet_size);
}

void ClientNetwork::sendInitUpdate()
{
    // send action packet
    const unsigned int packet_size = sizeof(UpdateHeader);
    char packet_data[packet_size];

    UpdateHeader header;
    header.update_type = INIT_CONNECTION;

    serialize(&header, packet_data);

	NetworkServices::sendMessage(ConnectSocket, packet_data, packet_size);
}

void ClientNetwork::sendReplaceCounterUpdate(ReplaceCounterUpdate replace_counter_update)
{
    const unsigned int packet_size = sizeof(UpdateHeader) + sizeof(IncreaseCounterUpdate);
    char packet_data[packet_size];

    UpdateHeader header;
    header.update_type = REPLACE_COUNTER;

    serialize(&header, packet_data);
    serialize(&replace_counter_update, packet_data + sizeof(UpdateHeader));

    NetworkServices::sendMessage(ConnectSocket, packet_data, packet_size);
}

void ClientNetwork::receiveUpdates() {
	int data_length = receivePackets(network_data);

    if (data_length <= 0)
    {
        //no data recieved
        return;
    }

    unsigned int i = 0;
    while (i < data_length)
    {
        UpdateHeader update_header;
        deserialize(&update_header, &(network_data[i]));
        unsigned int data_loc = i + sizeof(UpdateHeader);
        unsigned int update_length = update_type_data_lengths.at(update_header.update_type);

        switch (update_header.update_type) {

        case ISSUE_IDENTIFIER:
            IssueIdentifierUpdate issue_identifier_update;
            deserialize(&issue_identifier_update, &(network_data[data_loc]));

            game->handleIssueIdentifier(issue_identifier_update);
            break;

        case SERVER_TO_CLIENT:
			ServerToClientPacket updatePacket;
			deserialize(&updatePacket, &(network_data[data_loc]));

            game->handleServerActionEvent(updatePacket);
            break;

        default:
            std::cout << "Error in packet types" << std::endl;
            // This should never happen, so assert false so we find out if it does
            assert(false);
        }
        i += sizeof(UpdateHeader) + update_length;
    }
}

ClientNetwork::ClientNetwork(ClientGame* _game) {

	game=_game;

	#if defined(_WIN32)
	// create WSADATA object
	WSADATA wsaData;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0) {
		std::printf("WSAStartup failed with error: %d\n", iResult);
		exit(EXIT_FAILURE);
	}
	#endif

	// socket
	ConnectSocket = INVALID_SOCKET;

	// holds address info for socket to connect to
	struct addrinfo* result = NULL;\
	struct addrinfo* ptr = NULL;
    struct addrinfo hints;

	// set address info
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;  // TCP connection!!!

	//resolve server address and port
	iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
	std::cout << "Sending to " << SERVER_IP << std::endl;

	if (iResult != 0)
	{
		std::printf("getaddrinfo failed with error: %d\n", iResult);
		WSACLEANUP();
		exit(EXIT_FAILURE);
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);

        if (ISINVALIDSOCKET(ConnectSocket)) {
			std::printf("socket failed with error: %ld\n", GETSOCKETERRNO());
			WSACLEANUP();
			exit(EXIT_FAILURE);
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

		if (iResult == SOCKET_ERROR)
		{
			CLOSESOCKET(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			std::printf("The server is down... did not connect");
		}
	}



	// no longer need address info for server
	freeaddrinfo(result);



	// if connection failed
	if (ISINVALIDSOCKET(ConnectSocket))
	{
		std::printf("Unable to connect to server!\n");
		WSACLEANUP();
		exit(EXIT_FAILURE);
	}

	// Set the mode of the socket to be nonblocking
	u_long iMode = 1;

	#if defined(_WIN32)
	iResult = ioctlsocket(ConnectSocket, FIONBIO, &iMode);
	#else
	iResult = fcntl(ConnectSocket, O_NONBLOCK, O_NONBLOCK);
	#endif

	if (iResult == SOCKET_ERROR)
	{
		std::printf("ioctlsocket failed with error: %d\n", GETSOCKETERRNO());
		CLOSESOCKET(ConnectSocket);
		WSACLEANUP();
		exit(EXIT_FAILURE);
	}

	//disable nagle
	char value = 1;
	setsockopt(ConnectSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

}

int ClientNetwork::receivePackets(char* recvbuf)
{
	iResult = NetworkServices::receiveMessage(ConnectSocket, recvbuf, MAX_PACKET_SIZE);

	if (iResult == 0)
	{
		std::printf("Connection closed\n");
		CLOSESOCKET(ConnectSocket);
		WSACLEANUP();
		exit(EXIT_FAILURE);
	}

	return iResult;
}

ClientNetwork::~ClientNetwork(void) {

}
