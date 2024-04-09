// server.cpp : Defines the entry point for the application.
//

#include "Server.h"

void serverLoop();
std::unique_ptr<ServerGame> server;

int main()
{
	std::cout << "Hello, I'm the server." << std::endl;

    // initialize the server
    server = std::make_unique<ServerGame>();
    serverLoop();

	return 0;
}

void serverLoop()
{
    while (true)
    {
        Sleep(100);
        server->update();
    }
}

