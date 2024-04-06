// server.cpp : Defines the entry point for the application.
//

#include "server.h"

using namespace std;
void serverLoop();
ServerGame* server;

int main()
{
	cout << "Hello, I'm the server." << endl;

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

