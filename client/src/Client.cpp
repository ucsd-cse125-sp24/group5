// client.cpp : Defines the entry point for the application.
//
#include <chrono>
#include <thread>
#include "Client.h"

std::unique_ptr<ClientGame> client;
sge::ModelComposite *ptr;

int main()
{
	std::cout << "Hello, I'm the client." << std::endl;

//    client = std::make_unique<ClientGame>();
    sge::sgeInit();
    sge::ModelComposite m("./model/43-obj/obj/Wolf_One_obj.obj"); // this is here for testing purposes (for now)
    ptr = &m;
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
        ptr->render();
        glfwSwapBuffers(sge::window);
        std::this_thread::sleep_for(std::chrono::nanoseconds(10000));
    }
}
