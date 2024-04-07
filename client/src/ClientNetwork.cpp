#include "ClientNetwork.h"



ClientNetwork::ClientNetwork(void) {

	#if defined(_WIN32)
	// create WSADATA object
	WSADATA wsaData;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		exit(1);
	}
	#endif

	// socket
	ConnectSocket = INVALID_SOCKET;

	// holds address info for socket to connect to
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	// set address info
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;  // TCP connection!!!

	//resolve server address and port
	iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
	std::cout << "Sending to " << SERVER_IP << std::endl;

	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACLEANUP();
		exit(1);
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);

        if (ISINVALIDSOCKET(ConnectSocket)) {
			printf("socket failed with error: %ld\n", GETSOCKETERRNO());
			WSACLEANUP();
			exit(1);
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

		if (iResult == SOCKET_ERROR)
		{
			CLOSESOCKET(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			printf("The server is down... did not connect");
		}
	}



	// no longer need address info for server
	freeaddrinfo(result);



	// if connection failed
	if (ISINVALIDSOCKET(ConnectSocket))
	{
		printf("Unable to connect to server!\n");
		WSACLEANUP();
		exit(1);
	}

	// Set the mode of the socket to be nonblocking
	u_long iMode = 1;

	#if defined(_WIN32)
	iResult = ioctlsocket(ConnectSocket, FIONBIO, &iMode);
	#else
	iResult = fcntl(ConnectSocket, O_NONBLOCK, O_NONBLOCK);
	#endif

	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %d\n", GETSOCKETERRNO());
		CLOSESOCKET(ConnectSocket);
		WSACLEANUP();
		exit(1);
	}

	//disable nagle
	char value = 1;
	setsockopt(ConnectSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

}

int ClientNetwork::receivePackets(char* recvbuf)
{
	iResult = NetworkServices::receiveMessage(ConnectSocket, recvbuf, MAX_PACKET_SIZE);

	if (iResult == 0)
	{
		printf("Connection closed\n");
		CLOSESOCKET(ConnectSocket);
		WSACLEANUP();
		exit(1);
	}

	return iResult;
}