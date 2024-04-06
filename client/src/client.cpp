// client.cpp : Defines the entry point for the application.
//

#include "client.h"

using namespace std;
void clientLoop(void);
ClientGame* client;


int main()
{
	cout << "Hello, I'm the client." << endl;


    client = new ClientGame();
    clientLoop();
	return 0;
}


void clientLoop()
{
    while (true)
    {
        //do game stuff
        client->update();
    }
}