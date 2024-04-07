// server.cpp : Defines the entry point for the application.
//

#include "server.h"

void serverLoop();
ServerGame* server;

int main()
{
	std::cout << "Hello, I'm the server." << std::endl;

    // initialize the server
    server = new ServerGame();
    serverLoop();

	return 0;
}

void serverLoop()
{
    while (true)
    {
        server->update();
    }
}

