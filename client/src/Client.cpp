// client.cpp : Defines the entry point for the application.
//
#include <chrono>
#include <thread>
#include "Client.h"

std::unique_ptr<ClientGame> clientGame;
sge::ModelComposite *ptr; // TODO: Delete this later, here for debugging graphics engine

int main()
{
    std::cout << "Hello, I'm the client." << std::endl;

//    client = std::make_unique<ClientGame>();
    sge::sgeInit();
    // comment out ModelComposite stuff if you're debugging networking
    sge::ModelComposite object("rock_w_tex/rock2.obj"); // this is here for testing purposes (for now)
    ptr = &object;
    clientGame = std::make_unique<ClientGame>();
    clientLoop();
    glfwTerminate();
	return 0;
}

// macro for sleep; probably shouldn't be here but I'm lazy
void sleep(int ms) {
    #if defined(_WIN32)
    Sleep(ms);
    #else
    usleep(1000*ms);
    #endif
}

void clientLoop()
{
    ///////////// Graphics set up stuffs above^ /////////////

    // Main loop
    while (!glfwWindowShouldClose(sge::window))
    {
        // Poll for and process events (e.g. keyboard input callbacks)
        glfwPollEvents();

        // Send these input to server
        clientGame->sendClientInputToServer();

        // Receive updates from server
        clientGame->network->receiveUpdates();

        // Update local game state

        // Render
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ptr->render();
        // Swap buffers
        glfwSwapBuffers(sge::window);
    }

    // Terminate GLFW
    glfwTerminate();
    std::cout << "Good bye --- client terminal.\n";
    return;
}