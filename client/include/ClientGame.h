#ifndef ClientGameH
#define ClientGameH
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
#include "ClientNetwork.h"
#include "NetworkData.h"

// to avoid circular dependency
class ClientNetwork;

class ClientGame
{

public:

    ClientGame();
    ~ClientGame(void);
    std::unique_ptr<ClientNetwork> network;

    void handleActionEvent();
    void handleIssueIdentifier(IssueIdentifierUpdate issue_identifier_update);
    void handleReportCounter(ReportCounterUpdate report_counter_update);

    void update();

    int client_id;

private:
    int counter_start;
};

#endif