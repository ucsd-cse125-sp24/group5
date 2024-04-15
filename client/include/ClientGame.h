#pragma once
#if defined(_WIN32)
#pragma once
#include <WinSock2.h>
#include <windows.h>
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

#include <memory>
#include <cassert>
#include "ClientNetwork.h"
#include "NetworkData.h"

// to avoid circular dependency
class ClientNetwork;

class ClientGame
{

public:

    ClientGame();
    ~ClientGame(void);
    std::unique_ptr<ClientNetwork> network;

    void handleServerActionEvent();
    void handleIssueIdentifier(IssueIdentifierUpdate issue_identifier_update);

    void update(); // <- will need to break this into 1.receiving from network and 2.sending client input to network

    void sendClientInputToServer();

    int client_id;

    // Game movements requested from client's input
    bool requestForward;
    bool requestBackward;
    bool requestLeftward;
    bool requestRightward;
    bool requestJump;

    double yaw, pitch;  // initially both 0s. i.e. looking at initial 'forward' direction (negative z-axis) // in degrees
};