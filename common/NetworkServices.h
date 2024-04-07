#if defined(_WIN32)
#pragma once
#include <winsock2.h>
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>   // Needed for close()
#include <netdb.h>    // Needed for getaddrinfo() and freeaddrinfo()
#include <errno.h>
#endif

#if defined(_WIN32)
//Windows already defines SOCKET
#define ISINVALIDSOCKET(s) ((s) == INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())
#define WSACLEANUP() (WSACleanup())
#else
#define SOCKET int
#define ISINVALIDSOCKET(s) ((s) < 0)
#define CLOSESOCKET(s) close(s)
#define GETSOCKETERRNO() (errno)
#define WSACLEANUP()
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#endif

class NetworkServices
{

public:

	static int sendMessage(SOCKET curSocket, char* message, int messageSize);
	static int receiveMessage(SOCKET curSocket, char* buffer, int bufSize);

};