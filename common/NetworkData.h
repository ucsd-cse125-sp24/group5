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