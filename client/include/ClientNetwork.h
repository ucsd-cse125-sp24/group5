// Networking libraries
#pragma once
#if defined(_WIN32)
#pragma once
#include <WinSock2.h>
#include <windows.h>
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

#include <iostream>

#include "NetworkServices.h"
#include "NetworkData.h"
#include "SetupParser.h"

// to avoid circular dependency
class ClientGame;
#include "ClientGame.h"

// size of our buffer
#define DEFAULT_BUFLEN 512


#if defined(_WIN32)
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#endif

class ClientNetwork
{

public:

    // for error checking function calls in Winsock library
    int iResult;

    char network_data[MAX_PACKET_SIZE];

    // socket for client to connect to server
    SOCKET ConnectSocket;

    ClientGame *game;

    // receive updates
    void receiveUpdates();

    // for lobby send and receive update
    void sendLobbyClientToServer(LobbyClientToServerPacket& packet);

    // send each type of update
    void sendActionUpdate();
    void sendIncreaseCounterUpdate(IncreaseCounterUpdate& increase_counter_update);
    void sendReplaceCounterUpdate(ReplaceCounterUpdate& replace_counter_update);
    void sendClientToServerPacket(ClientToServerPacket& packet);
    void sendInitUpdate();

    // ctor/dtor
    ClientNetwork(ClientGame* game);
    ~ClientNetwork(void);

    int receivePackets(char*);
};