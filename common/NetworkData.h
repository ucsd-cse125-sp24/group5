#pragma once

#include <string>
#include <map>

#define MAX_PACKET_SIZE 1000000

#define MAX_CONTENTS_SIZE 128

enum UpdateTypes {
    // sent by a client when it first connects to the server
    INIT_CONNECTION = 0,

    // for testing purposes. sent back and forth between client and server
    ACTION_EVENT = 1,

    // sent by a client to increase their counter
    INCREASE_COUNTER = 2,

    // sent by the server to tell the client its current counter value
    REPORT_COUNTER = 4,

    // sent by a client to replace their counter's value with the provided value
    REPLACE_COUNTER = 5,
};

struct IncreaseCounterUpdate {
    int add_amount;
};

struct ReportCounterUpdate {
    int counter_value;
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
    {REPLACE_COUNTER,sizeof(ReplaceCounterUpdate)},
    {REPORT_COUNTER,sizeof(ReportCounterUpdate)}
};

// copy the information from the struct into data
template <typename T> void serialize(T* struct_ptr, char* data) {
    std::memcpy(data, struct_ptr, sizeof(T));
}

// copy the information from data into the struct
template <typename T> void deserialize(T* struct_ptr, char* data) {
    std::memcpy(struct_ptr, data, sizeof(T));
}