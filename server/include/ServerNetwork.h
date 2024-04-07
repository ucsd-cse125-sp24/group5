#if defined(_WIN32)
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>  // Needed for TCP_NODELAY
#include <arpa/inet.h>
#include <unistd.h>   // Needed for close()
#include <netdb.h>    // Needed for getaddrinfo() and freeaddrinfo()
#include <fcntl.h>    // Needed for fcntl
#include <errno.h>
#endif

#include <vector>
#include <iostream>
#include "NetworkServices.h"
#include "NetworkData.h"

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "6881"

class ServerNetwork
{

public:

    ServerNetwork(void);
    ~ServerNetwork(void);

    // Socket to listen for new connections
    SOCKET ListenSocket;

    // Socket to give to the clients
    SOCKET ClientSocket;

    // for error checking return values
    int iResult;

    // table to keep track of each client's socket
    std::vector<SOCKET> sessions;
    
    // accept new connections
    bool acceptNewClient(unsigned int& id);

    // receive incoming data
    int receiveData(unsigned int client_id, char* recvbuf);

    // send data to all clients
    void sendToAll(char* packets, int totalSize);
};
