#include "ServerNetwork.h"



ServerNetwork::ServerNetwork(void)
{

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

        // insert new client into session id table
        sessions[id] = ClientSocket;
        return true;
    }

    return false;
}

// receive incoming data
int ServerNetwork::receiveData(unsigned int client_id, char* recvbuf)
{
    if (client_id < sessions.size())
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

// send data to all clients
void ServerNetwork::sendToAll(char* packets, int totalSize)
{
    SOCKET currentSocket;
    int iSendResult;

    for (unsigned int i = 0; i < sessions.size(); i++)
    {
        currentSocket = sessions[i];
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
