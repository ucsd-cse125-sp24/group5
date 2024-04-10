#pragma once

#include <string>

#define MAX_PACKET_SIZE 1000000

#define MAX_CONTENTS_SIZE 128

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

struct IncreaseCounterPacketContents {
    int add_amount;
};

struct ReportCounterPacketContents {
    int counter_value;
};

struct Packet {

    unsigned int packet_type;

    char contents_data[MAX_CONTENTS_SIZE];

    void serialize(char* data) {
        std::memcpy(data, this, sizeof(Packet));
    }

    void deserialize(char* data) {
        std::memcpy(this, data, sizeof(Packet));
    }
};

template <typename T> void serialize(T* packet_contents, char* data)
{
    std::memcpy(data, packet_contents, sizeof(T));
}

template <typename T> void deserialize(T* packet_contents, char* data)
{
    std::memcpy(packet_contents, data, sizeof(T));
}