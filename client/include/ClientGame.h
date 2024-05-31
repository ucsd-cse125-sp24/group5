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
#include <vector>
#include <deque>
#include "ClientNetwork.h"
#include "NetworkData.h"
#include "sound/SoundManager.h"
#include "ui/UIManager.h"
#include "sge/GraphicsEntity.h"
#include <glm/glm.hpp>

#define EGG_POSITION_INDEX NUM_PLAYER_ENTITIES


// to avoid circular dependency
class ClientNetwork;

enum PlayerAnimations {
    NO_ANIMATION = -1,
    SHOOTING = 0,
    JUMPING = 1,
    STILL = 2,
    WALKING = 3,
};

#define BULLET_SEGMENT_PORTION 0.5f
struct BulletToRender {
    BulletToRender(glm::vec3 start, glm::vec3 end, int framesToRender) : start(start), currEnd(start), framesToRender(framesToRender) {
        currEnd = start + (end-start) * BULLET_SEGMENT_PORTION;
        delta = (end - currEnd) / (float)(framesToRender);
    }
    glm::vec3 start;
    glm::vec3 delta;
    glm::vec3 currEnd;
    int framesToRender;  // start at BULLET_FRAMES, then --, -- ...
};

class ClientGame
{

public:

    ClientGame();
    ~ClientGame(void);
    std::unique_ptr<ClientNetwork> network;

    void handleServerActionEvent(ServerToClientPacket& updatePacket);
    void handleIssueIdentifier(IssueIdentifierUpdate issue_identifier_update);
    void handleBulletPacket(BulletPacket& bulletPacket);
    void updateShootingEmo();
    void updateBulletQueue();

    void updateAnimations(std::bitset<NUM_STATES> movementEntityStates[]);

    void update(); // <- will need to break this into 1.receiving from network and 2.sending client input to network

    void sendClientInputToServer();

    void sendLobbySelectionToServer(int browsingCharacterUID, int selectedCharacterUID);
    void handleLobbySelectionPacket(LobbyServerToClientPacket& lobbyPacket);

    int client_id = 0;  // for init only, will be overwritten when the server assign me a client_id

    // Game movements requested from client's input
    bool requestForward = false;
    bool requestBackward = false;
    bool requestLeftward = false;
    bool requestRightward = false;
    bool requestJump = false;
    bool requestThrowEgg = false;

    bool requestShoot = false;
    bool requestAbility = false;

    float playerYaw = -90.0f; // init to -90 so that default direction is -z axis.
    float playerPitch = 0.0f;

    // Game world data (local + received from the server)
    glm::vec3 positions[NUM_MOVEMENT_ENTITIES];
    float yaws[NUM_MOVEMENT_ENTITIES];
    float pitches[NUM_MOVEMENT_ENTITIES];
    float cameraDistances[NUM_MOVEMENT_ENTITIES];
    int gameSeason;

    std::deque<BulletToRender> bulletQueue;
    int shootingEmo = 0;

    int animations[NUM_MOVEMENT_ENTITIES];

    // Contains the indices between 0 and NUM_MOVEMENT_ENTITIES which correspond to players
    std::vector<unsigned int> playerIndices;


    // map player's id to entity you want
    // player's character selection
    int characterUID[NUM_PLAYER_ENTITIES] = { NO_CHARACTER,NO_CHARACTER,NO_CHARACTER,NO_CHARACTER };

    int browsingCharacterUID[NUM_PLAYER_ENTITIES];

    // teammate setup
    int teams[NUM_PLAYER_ENTITIES] = { 0,0,0,0 };
};