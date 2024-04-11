#pragma once

#include <string>

#define MAX_PACKET_SIZE 1000000

enum PacketTypes {

    INIT_CONNECTION = 0,

    ACTION_EVENT = 1,

};

struct Packet {

    unsigned int packet_type;

    void serialize(char* data) {
        std::memcpy(data, this, sizeof(Packet));
    }

    void deserialize(char* data) {
        std::memcpy(this, data, sizeof(Packet));
    }
};


struct ClientToServerActionPacket {
    unsigned int packet_type = ACTION_EVENT;

    // Movement requests
    bool requestForward;
    bool requestBackward;
    bool requestLeftward;
    bool requestRightward;
    bool requestJump;

    // (todo: other requests, e.g. shooting, skill)


    void serialize(char* data) {
        std::memcpy(data, this, sizeof(ClientToServerActionPacket));
    }

    void deserialize(char* data) {
        std::memcpy(this, data, sizeof(ClientToServerActionPacket));
    }
};