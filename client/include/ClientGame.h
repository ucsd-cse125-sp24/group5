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

    void initializeParticleEmitters();

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
    // make sure these 3 below are using NUM_PLAYER_ENTITIES!!!
    float cameraDistances[NUM_PLAYER_ENTITIES];
    int healths[NUM_PLAYER_ENTITIES];
    int scores[NUM_PLAYER_ENTITIES];
    Season currentSeason = SPRING_SEASON;
    float seasonBlend = 0;

    bool projActive[NUM_TOTAL_PROJECTILES];
    std::unique_ptr<sge::DiskParticleEmitterEntity> ambientParticleEmitters[4];
    std::vector<glm::vec4> ambientStartingColors[4] = {
        std::vector<glm::vec4>({
            glm::vec4(0.54f, 0.1f, 0.39f, 1.0f)
        }),
        std::vector<glm::vec4>({
            glm::vec4(0.59f, 0.82f, 0.294f, 1.0f)
        }),
        std::vector<glm::vec4>({
            glm::vec4(0.9f, 0.56f, 0.125f, 1.0f)
        }),
        std::vector<glm::vec4>({
            glm::vec4(0.83f, 0.925f, 0.98f, 1.0f)
        })
    };
    std::vector<glm::vec4> ambientEndingColors[4] = {
        std::vector<glm::vec4>({
            glm::vec4(1.0f, 0.66f, 0.89f, 0.5f)
        }),
        std::vector<glm::vec4>({
            glm::vec4(0.84f, 1.0f, 0.63f, 0.5f)
        }),
        std::vector<glm::vec4>({
            glm::vec4(0.98f, 0.76f, 0.24f, 0.5f)
        }),
        std::vector<glm::vec4>({
            glm::vec4(1.0f, 1.0f, 1.0f, 0.5f)
        })
    };
    std::vector<float> ambientColorProbs[4] = { 
        std::vector<float>({ 1.0f }),
        std::vector<float>({ 1.0f }),
        std::vector<float>({ 1.0f }),
        std::vector<float>({ 1.0f })
    };

    std::unique_ptr<sge::ParticleEmitterEntity> projParticleEmitters[NUM_TOTAL_PROJECTILES];
    std::unique_ptr<sge::ParticleEmitterEntity> projExplosionEmitters[NUM_TOTAL_PROJECTILES];
    std::vector<glm::vec4> projStartingColors[NUM_PROJ_TYPES] = {
        std::vector<glm::vec4>({
            glm::vec4(0.12f, 0.42f, 0.12f, 1.0f),
            glm::vec4(0.1f, 0.57f, 0.27f, 1.0f),
            glm::vec4(0.54f, 0.1f, 0.39f, 1.0f)
        }),
        std::vector<glm::vec4>({ 
            glm::vec4(0.83f, 0.72f, 0.02f, 1.0f), 
            glm::vec4(0.84f, 0.59f, 0.02f, 1.0f), 
            glm::vec4(0.28f, 0.53f, 0.05f, 1.0f)
        }),
        std::vector<glm::vec4>({
            glm::vec4(0.68f, 0.31f, 0.04f, 1.0f),
            glm::vec4(0.67f, 0.16f, 0.03f, 1.0f),
            glm::vec4(0.90f, 0.42f, 0.02f, 1.0f)
        }),
        std::vector<glm::vec4>({
            glm::vec4(0.48f, 0.80f, 0.90f, 1.0f),
            glm::vec4(0.77f, 0.81f, 0.82f, 1.0f),
            glm::vec4(0.48f, 0.37f, 0.73f, 1.0f)
        })
    };
    std::vector<glm::vec4> projEndingColors[NUM_PROJ_TYPES] = { 
        std::vector<glm::vec4>({ 
            glm::vec4(0.29f, 0.69f, 0.29f, 0.0f), 
            glm::vec4(0.43f, 1.0f, 0.64f, 0.0f), 
            glm::vec4(1.0f, 0.66f, 0.89f, 0.0f)}),
        std::vector<glm::vec4>({
            glm::vec4(0.98f, 0.91f, 0.47f, 0.0f),
            glm::vec4(0.98f, 0.82f, 0.46f, 0.0f),
            glm::vec4(0.53f, 0.84f, 0.24f, 0.0f)}),
        std::vector<glm::vec4>({
            glm::vec4(0.94f, 0.56f, 0.26f, 0.0f),
            glm::vec4(0.96f, 0.35f, 0.17f, 0.0f),
            glm::vec4(1.0f, 0.8f, 0.08f, 0.0f)}),
        std::vector<glm::vec4>({
            glm::vec4(0.82f, 0.93f, 0.97f, 0.0f),
            glm::vec4(0.94f, 0.96f, 0.96f, 0.0f),
            glm::vec4(0.90f, 0.86f, 0.99f, 0.0f)})
    };
    std::vector<float> projColorProbs[NUM_PROJ_TYPES] = { 
        std::vector<float>({ 0.33f, 0.33f, 0.34f }),
        std::vector<float>({ 0.33f, 0.33f, 0.34f }),
        std::vector<float>({ 0.33f, 0.33f, 0.34f }), 
        std::vector<float>({ 0.33f, 0.33f, 0.34f })
    };

    std::deque<BulletToRender> bulletQueue;
    int shootingEmo = 0;

    int animations[NUM_MOVEMENT_ENTITIES];

    // Contains the indices between 0 and NUM_MOVEMENT_ENTITIES which correspond to players
    std::vector<unsigned int> playerIndices;

    // Contains the indices between 0 and NUM_MOVEMENT_ENTITIES which correspond to projectiles
    std::vector<unsigned int> projIndices;

    // map player's id to entity you want
    // player's character selection
    int characterUID[NUM_PLAYER_ENTITIES] = { NO_CHARACTER,NO_CHARACTER,NO_CHARACTER,NO_CHARACTER };

    int browsingCharacterUID[NUM_PLAYER_ENTITIES];

    // teammate setup
    int teams[NUM_PLAYER_ENTITIES] = { 0,0,0,0 };
};
