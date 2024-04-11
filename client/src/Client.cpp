// client.cpp : Defines the entry point for the application.
//

#include "Client.h"

std::unique_ptr<ClientGame> clientGame;
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// Function to handle resizing of the window
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    std::cout << "Hello, I'm the client." << std::endl;

    // Connect to server
    clientGame = std::make_unique<ClientGame>();

    ///////////// Graphics set up stuffs below /////////////
    glm::mat4 m;
    // Initialize GLFW
    std::cout << "sup adsfa;lsdkjfaskdl;fj\n";
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a GLFW window
    GLFWwindow *window = glfwCreateWindow(800, 600, "GLFW/GLEW Test", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set the viewport size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

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

        // Update local game state

        // Render
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red background
        glClear(GL_COLOR_BUFFER_BIT);

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Terminate GLFW
    glfwTerminate();
    std::cout << "Good bye --- client terminal.\n";
    return 0;
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