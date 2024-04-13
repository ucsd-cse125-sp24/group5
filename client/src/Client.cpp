// client.cpp : Defines the entry point for the application.
//
#include <chrono>
#include <thread>
#include "Client.h"

std::unique_ptr<ClientGame> clientGame;
sge::ModelComposite *ptr; // TODO: Delete this later, here for debugging graphics engine
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void clientLoop(void);

// Function to handle resizing of the window
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    std::cout << "Hello, I'm the client." << std::endl;

//    client = std::make_unique<ClientGame>();
    sge::sgeInit();
    sge::ModelComposite m("C:\\Users\\benjx\\OneDrive - UC San Diego\\Documents\\Classwork\\Y3Q3_SP24\\CSE125\\group5\\client\\model\\19-obj\\obj\\Only_Spider_with_Animations_Export.obj"); // this is here for testing purposes (for now)
    ptr = &m;
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
    // Register callback for window resizing
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Register keyboard input callbacks
    glfwSetKeyCallback(window, key_callback);

    ///////////// Graphics set up stuffs above^ /////////////

    // Main loop
    while (!glfwWindowShouldClose(window))
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
        glClear(GL_COLOR_BUFFER_BIT);
        ptr->render();
        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Terminate GLFW
    glfwTerminate();
    std::cout << "Good bye --- client terminal.\n";
    return;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // WASD + space Movements
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_W:
            clientGame->requestForward = true;
            break;
        case GLFW_KEY_A:
            clientGame->requestLeftward = true;
            break;
        case GLFW_KEY_S:
            clientGame->requestBackward = true;
            break;
        case GLFW_KEY_D:
            clientGame->requestRightward = true;
            break;
        case GLFW_KEY_SPACE:
            clientGame->requestJump = true;
            break;
        default:
            std::cout << "unrecognized key press, gg\n";
            break;
        }
    }
    else if (action == GLFW_RELEASE) {
        switch (key)
        {
        case GLFW_KEY_W:
            clientGame->requestForward = false;
            break;
        case GLFW_KEY_A:
            clientGame->requestLeftward = false;
            break;
        case GLFW_KEY_S:
            clientGame->requestBackward = false;
            break;
        case GLFW_KEY_D:
            clientGame->requestRightward = false;
            break;
        case GLFW_KEY_SPACE:
            clientGame->requestJump = false;
            break;
        default:
            std::cout << "unrecognized key release, gg\n";
            break;
        }
    }



}