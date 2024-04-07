#if defined(_WIN32)
#pragma once
#include <winsock2.h>
#include <Windows.h>
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

#include "ClientNetwork.h"
#include "NetworkData.h"

class ClientGame
{

public:

    ClientGame();
    ~ClientGame(void);

    ClientNetwork* network;

    void sendActionPackets();

    char network_data[MAX_PACKET_SIZE];

    void update();
};

