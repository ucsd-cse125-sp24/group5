#pragma once

#include <string>
#include <map>
#include <bitset>
#include <glm/glm.hpp>
#include "GameConstants.h"

#define MAX_PACKET_SIZE 1000000

#define MAX_CONTENTS_SIZE 128

enum UpdateTypes {
    // sent by a client when it first connects to the server
    INIT_CONNECTION = 0,

    // sent by the server to a new client after it receives an INIT_CONNECTION
    // gives the client a number so it can tell what packets are about it
    ISSUE_IDENTIFIER = 1,

    // for testing purposes. sent back and forth between client and server
    ACTION_EVENT = 2,

    // sent by a client to increase their counter
    INCREASE_COUNTER = 3,

    // sent by the server to tell the client its current counter value
    // REPORT_COUNTER = 4,

    // sent by a client to replace their counter's value with the provided value
    REPLACE_COUNTER = 5,

    CLIENT_TO_SERVER = 6,

    SERVER_TO_CLIENT = 7,

    BULLETS = 8,
};

struct IncreaseCounterUpdate {
    int add_amount;
};

struct IssueIdentifierUpdate {
    int client_id;
};

struct ReportCounterUpdate {
    int counter_value;
    int client_id;
};

struct ClientToServerPacket {

    unsigned int packet_type; // don't need this

    // Movement requests
    bool requestForward;
    bool requestBackward;
    bool requestLeftward;
    bool requestRightward;
    bool requestJump;
    bool requestThrowEgg;

    // shooting and ability
    bool requestShoot;
    bool requestAbility;

    // Movement angle
    float yaw, pitch;
};

/* Below are server-to-client packets... terrible naming, yes...*/

struct ServerToClientPacket {
    // to every client, for all clients

    glm::vec3 positions[NUM_MOVEMENT_ENTITIES];
    float yaws[NUM_MOVEMENT_ENTITIES];
    float pitches[NUM_MOVEMENT_ENTITIES];
    float cameraDistances[NUM_PLAYER_ENTITIES];
    std::bitset<NUM_STATES> movementEntityStates[NUM_MOVEMENT_ENTITIES];
    int healths[NUM_PLAYER_ENTITIES];
    int scores[NUM_PLAYER_ENTITIES];
    int currentSeason;
    bool eggIsDanceBomb;
};

struct BulletTrail {
    int shooterId;
    glm::vec3 start;
    glm::vec3 end;
    int playerHit;
};

struct BulletPacket {
    unsigned int count;
    BulletTrail bulletTrail[NUM_PLAYER_ENTITIES];
};

struct ReplaceCounterUpdate {
    int counter_value;
};

struct UpdateHeader {
    unsigned int update_type;
};

const std::map<unsigned int, unsigned int> update_type_data_lengths = { 
    {INIT_CONNECTION,0},
    {ACTION_EVENT,0},
    {INCREASE_COUNTER,sizeof(IncreaseCounterUpdate)},
    {ISSUE_IDENTIFIER,sizeof(IssueIdentifierUpdate)},
    {REPLACE_COUNTER,sizeof(ReplaceCounterUpdate)},
    {CLIENT_TO_SERVER,sizeof(ClientToServerPacket)},
    {SERVER_TO_CLIENT, sizeof(ServerToClientPacket)},
    {BULLETS,           sizeof(BulletPacket)},
};

// copy the information from the struct into data
template <typename T> void serialize(T* struct_ptr, char* data) {
    std::memcpy(data, struct_ptr, sizeof(T));
}

// copy the information from data into the struct
template <typename T> void deserialize(T* struct_ptr, char* data) {
    std::memcpy(struct_ptr, data, sizeof(T));
}
