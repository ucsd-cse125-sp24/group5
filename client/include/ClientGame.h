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
#include <glm/glm.hpp>

#define EGG_POSITION_INDEX NUM_PLAYER_ENTITIES


// to avoid circular dependency
class ClientNetwork;

class ClientGame
{

public:

    ClientGame();
    ~ClientGame(void);
    std::unique_ptr<ClientNetwork> network;

    void handleServerActionEvent(ServerToClientPacket& updatePacket);
    void handleIssueIdentifier(IssueIdentifierUpdate issue_identifier_update);

    void update(); // <- will need to break this into 1.receiving from network and 2.sending client input to network

    void sendClientInputToServer();

    int client_id = 0;  // for init only, will be overwritten when the server assign me a client_id

    // Game movements requested from client's input
    bool requestForward = false;
    bool requestBackward = false;
    bool requestLeftward = false;
    bool requestRightward = false;
    bool requestJump = false;

    float playerYaw = -90.0f; // init to -90 so that default direction is -z axis.
    float playerPitch = 0.0f;

    // Game world data (local + received from the server)
    glm::vec3 positions[NUM_MOVEMENT_ENTITIES];
    float yaws[NUM_MOVEMENT_ENTITIES];
    float pitches[NUM_MOVEMENT_ENTITIES];
};