#pragma once

#include <string>

#define MAX_PACKET_SIZE 1000000

enum PacketTypes {
    // sent by a client when it first connects to the server
    INIT_CONNECTION = 0,

    // for testing purposes. sent back and forth between client and server
    ACTION_EVENT = 1,

    // sent by a client to increase their counter
    // Data:
    // num_A: amount to increase the counter by
    INCREASE_COUNTER = 2,

    // sent by the server to tell the client its current counter value
    // Data:
    // num_A: current counter value
    REPORT_COUNTER = 4,
};

struct Packet {

    unsigned int packet_type;

    // The first integer to send in this packet (interpretation depends on packet_type)
    int num_A;

    void serialize(char* data) {
        std::memcpy(data, this, sizeof(Packet));
    }

    void deserialize(char* data) {
        std::memcpy(this, data, sizeof(Packet));
    }
};