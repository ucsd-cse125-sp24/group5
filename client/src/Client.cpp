// client.cpp : Defines the entry point for the application.
//

#include "Client.h"

std::unique_ptr<ClientGame> client;

int main()
{
	std::cout << "Hello, I'm the client." << std::endl;

//    client = std::make_unique<ClientGame>();
    sge::sgeInit();
    clientLoop();
    glfwTerminate();
	return 0;
}


void clientLoop()
{
    while (!glfwWindowShouldClose(sge::window))
    {
        //do game stuff
//        client->update();

        sge::sgeLoop();
    }
}
