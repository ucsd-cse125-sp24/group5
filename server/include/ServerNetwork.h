#pragma once
#if defined(_WIN32)
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

#include <map>
#include <iostream>
#include "NetworkServices.h"
#include "NetworkData.h"
class ServerGame;
#include "ServerGame.h"

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512

class ServerNetwork
{

public:

    ServerNetwork(ServerGame* _game);
    ~ServerNetwork(void);

    ServerGame* game;

    // all update sends
    void sendIssueIdentifierUpdate(IssueIdentifierUpdate& issue_identifier_update);

    void sendPositionsUpdates(ServerToClientPacket& packet);

    void sendBulletsUpdate(BulletPacket& packet);

    void sendGameEndData(GameEndPacket& packet);
    
    void sendCharacterSelectionUpdate(LobbyServerToClientPacket& packet);

    // Socket to listen for new connections
    SOCKET ListenSocket;

    // Socket to give to the clients
    SOCKET ClientSocket;

    // for error checking return values
    int iResult;

    // table to keep track of each client's socket
    std::map<unsigned int, SOCKET> sessions;

    // get all updates from clients
    void receiveFromClients();
    
    // accept new connections
    bool acceptNewClient(unsigned int& id);

    // receive incoming data
    int receiveData(unsigned int client_id, char* recvbuf);

    // issue client_id to individual client
    void sendToClient(unsigned int client_id, char* packets, int totalSize);

    // send data to all clients
    void sendToAll(char* packets, int totalSize);

private:
    // data buffer
    char network_data[MAX_PACKET_SIZE];
};