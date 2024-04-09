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
#include "ClientNetwork.h"
#include "NetworkData.h"

class ClientGame
{

public:

    ClientGame();
    ~ClientGame(void);
    std::unique_ptr<ClientNetwork> network;

    void sendActionPackets();

    void sendCounterIncrease();

    char network_data[MAX_PACKET_SIZE];

    void update();
};

