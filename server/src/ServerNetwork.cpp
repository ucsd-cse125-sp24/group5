#include "ServerNetwork.h"

// Send the issue identifier update to the associated client
// (assumes that issue_identifier_update.client_id tells us which client to send to as well)
void ServerNetwork::sendIssueIdentifierUpdate(IssueIdentifierUpdate issue_identifier_update) {
    const unsigned int packet_size = sizeof(UpdateHeader) + sizeof(IssueIdentifierUpdate);
    char packet_data[packet_size];

    UpdateHeader header;
    header.update_type = ISSUE_IDENTIFIER;

    serialize(&header, packet_data);
    serialize(&issue_identifier_update, packet_data + sizeof(UpdateHeader));

    sendToClient(issue_identifier_update.client_id, packet_data, packet_size);
}

void ServerNetwork::sendReportCounterUpdate(ReportCounterUpdate report_counter_update) {       
    // create packet with updated counter
    const unsigned int packet_size = sizeof(UpdateHeader) + sizeof(ReportCounterUpdate);
    char packet_data[packet_size];

    UpdateHeader header;
    header.update_type = REPORT_COUNTER;
    serialize(&header, packet_data);
    
    serialize(&report_counter_update, packet_data + sizeof(UpdateHeader));
    sendToAll(packet_data, packet_size);
}

void ServerNetwork::sendActionUpdate()
{
    // send action packet
    const unsigned int packet_size = sizeof(UpdateHeader);
    char packet_data[packet_size];

    UpdateHeader header;
    header.update_type = ACTION_EVENT;

    serialize(&header, packet_data);

    sendToAll(packet_data, packet_size);
}

void ServerNetwork::receiveFromClients()
{
    // go through all clients
    std::map<unsigned int, SOCKET>::iterator iter;

    for (iter = sessions.begin(); iter != sessions.end(); /* no increment*/) {
        int data_length = receiveData(iter->first, network_data);

        if (data_length == -1) {
            // waiting for msg, nonblocking
            iter++;
            continue;
        } else if (data_length == 0) {
            // no data recieved, ending session
            std::cout << "No data received (data_length=" << data_length << "), ending session.\n";
            sessions.erase(iter++);  // trick to remove while iterating
            continue;
        }

        unsigned int i = 0;
        while (i < data_length) {
            UpdateHeader update_header;
            deserialize(&update_header, &(network_data[i]));
            unsigned int data_loc = i + sizeof(UpdateHeader);
            unsigned int update_length = update_type_data_lengths.at(update_header.update_type);

            switch (update_header.update_type) {

            case INIT_CONNECTION:
                game->handleInitConnection(iter->first);
                break;

            case ACTION_EVENT:
                break;

            case INCREASE_COUNTER:
                IncreaseCounterUpdate increase_counter_update;
                deserialize(&increase_counter_update, &(network_data[data_loc]));

                game->handleIncreaseCounter(iter->first, increase_counter_update);
                break;

            case REPLACE_COUNTER:
                ReplaceCounterUpdate replace_counter_update;
                deserialize(&replace_counter_update, &(network_data[data_loc]));

                game->handleReplaceCounter(iter->first, replace_counter_update);
                break;

            default:
                std::cout << "Error in packet types" << std::endl;
                // This should never happen, so assert false so we find out if it does
                assert(false);

                break;
            }
            // Move on to the next update
            i += sizeof(UpdateHeader) + update_length;
        }
        iter++;
    }
}

ServerNetwork::ServerNetwork(ServerGame* _game)
{
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

    // our sockets for the server
    ListenSocket = INVALID_SOCKET;
    ClientSocket = INVALID_SOCKET;

    // address info for the server to listen to
    struct addrinfo* result = NULL;
    struct addrinfo hints;

    // set address information
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;    // TCP connection!!!
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);

    if (iResult != 0) {
        std::printf("getaddrinfo failed with error: %d\n", iResult);
        WSACLEANUP();
        exit(EXIT_FAILURE);
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ISINVALIDSOCKET(ListenSocket)) {
        std::printf("socket failed with error: %d\n", GETSOCKETERRNO());
        freeaddrinfo(result);
        WSACLEANUP();
        exit(EXIT_FAILURE);
    }

    // Set the mode of the socket to be nonblocking
    u_long iMode = 1;

	#if defined(_WIN32)
    iResult = ioctlsocket(ListenSocket, FIONBIO, &iMode);
    #else
    iResult = fcntl(ListenSocket, O_NONBLOCK, O_NONBLOCK);
    #endif

    if (iResult == SOCKET_ERROR) {
        std::printf("ioctlsocket failed with error: %d\n", GETSOCKETERRNO());
        CLOSESOCKET(ListenSocket);
        WSACLEANUP();
        exit(EXIT_FAILURE);
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);

    if (iResult == SOCKET_ERROR) {
        std::printf("bind failed with error: %d\n", GETSOCKETERRNO());
        freeaddrinfo(result);
        CLOSESOCKET(ListenSocket);
        WSACLEANUP();
        exit(EXIT_FAILURE);
    }

    // no longer need address information
    freeaddrinfo(result);

    // start listening for new clients attempting to connect
    iResult = listen(ListenSocket, SOMAXCONN);

    if (iResult == SOCKET_ERROR) {
        std::printf("listen failed with error: %d\n", GETSOCKETERRNO());
        CLOSESOCKET(ListenSocket);
        WSACLEANUP();
        exit(EXIT_FAILURE);
    }
}

// accept new connections
bool ServerNetwork::acceptNewClient(unsigned int& id)
{
    // get the client address
    struct sockaddr_in client_addr;
    socklen_t slen = sizeof(client_addr);

    // if client waiting, accept the connection and save the socket
    ClientSocket = accept(ListenSocket, (struct sockaddr*)&client_addr, &slen);

    if (!ISINVALIDSOCKET(ClientSocket))
    {
        // we will get the client IP here - will be used to identify client
        struct in_addr ipAddr = client_addr.sin_addr;
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
        std::string client_ip(str);
        std::cout << "Listen from " << client_ip << std::endl;

        //disable nagle on the client's socket
        char value = 1;
        setsockopt(ClientSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));


        // [Reconnection] - what if this ClientSocket was recently used by a client in the sessions table, but that client got disconnected for a little while
        // then reassign that client this ClientSocket. 
        // below: instead of passed in `id`, use the lost client_id (gotta find in sessions / tweak sessions structure)

        // Instead, we could mark sessions[id] as INVALID_SESSION. And then later, when someone reconnects, just assign him the INVALID_SESSION slot. 
        // Instead of map, an array is the best for speed (spatial locality). 

        // insert new client into session id table
        sessions[id] = ClientSocket;
        return true;
    }

    return false;
}

// receive incoming data
int ServerNetwork::receiveData(unsigned int client_id, char* recvbuf)
{
    if (sessions.count(client_id))
    {
        SOCKET currentSocket = sessions[client_id];
        iResult = NetworkServices::receiveMessage(currentSocket, recvbuf, MAX_PACKET_SIZE);
        /* Possible iResult return values:
            -1: waiting for msg to arrive from the nonblocking socket...
            0 : no data received, so close connection.
            n : received n bytes of data.
        */
        
        if (iResult == 0) 
        {
            std::printf("Connection closed\n");
            CLOSESOCKET(currentSocket);
            return 0;
        }

        return iResult;
    }
    return 0;
}

// send data to a specific client
void ServerNetwork::sendToClient(unsigned int client_id, char* packets, int totalSize)
{
    if (sessions.count(client_id))
    {
        SOCKET currentSocket = sessions[client_id];
        int iSendResult = NetworkServices::sendMessage(currentSocket, packets, totalSize);

        if (iSendResult == SOCKET_ERROR)
        {
            std::printf("send failed with error: %d\n", GETSOCKETERRNO());
            CLOSESOCKET(currentSocket);
        }
    }
    else {
        std::printf("send failed with error: client id %d is invalid\n", client_id);
    }
}

// send data to all clients
void ServerNetwork::sendToAll(char* packets, int totalSize)
{
    SOCKET currentSocket;
    std::map<unsigned int, SOCKET>::iterator iter;
    int iSendResult;

    for (iter = sessions.begin(); iter != sessions.end(); iter++)
    {
        currentSocket = iter->second;
        iSendResult = NetworkServices::sendMessage(currentSocket, packets, totalSize);

        if (iSendResult == SOCKET_ERROR)
        {
            std::printf("send failed with error: %d\n", GETSOCKETERRNO());
            CLOSESOCKET(currentSocket);
        }
    }
}

ServerNetwork::~ServerNetwork(void) {

}
