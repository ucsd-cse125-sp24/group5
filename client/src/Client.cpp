// client.cpp : Defines the entry point for the application.
//

#include "Client.h"
// #include <assimp/Importer.hpp>
// #include <assimp/scene.h>
// #include <assimp/postprocess.h>

std::unique_ptr<ClientGame> clientGame;
double lastX, lastY;    // last cursor position
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void cursor_callback(GLFWwindow* window,  double xpos, double ypos);
void clientLoop(void);

// Function to handle resizing of the window
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    std::cout << "Hello, I'm the client." << std::endl;

    // Assimp::Importer a;
    std::cout << "wassup\n";
    clientGame = std::make_unique<ClientGame>();
    clientLoop();
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
    ///////////// Graphics set up stuffs below /////////////
    glm::mat4 m;
    // Initialize GLFW
    std::cout << "sup adsfa;lsdkjfaskdl;fj\n";
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // Create a GLFW window
    GLFWwindow *window = glfwCreateWindow(800, 600, "GLFW/GLEW Test", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return;
    }

    // Set the viewport size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Register callback for window resizing
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Register keyboard input callbacks
    glfwSetKeyCallback(window, key_callback);
    // Register cursor input callbacks
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // virtual & unlimited cursor movement for camera control , will hide cursor!
    glfwGetCursorPos(window, &lastX, &lastY);     // init
    glfwSetCursorPosCallback(window, cursor_callback);

    ///////////// Graphics set up stuffs above^ /////////////

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll for and process events (e.g. keyboard & mouse input callbacks)
        glfwPollEvents();

        // Send these input to server
        clientGame->sendClientInputToServer();

        // Receive updates from server
        clientGame->network->receiveUpdates();

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


void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{

    double deltaX = xpos - lastX;
    double deltaY = lastY - ypos;  // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;
    std::printf("cursor moved right(%lf) up(%lf)\n", deltaX, deltaY);
    
    const double SENSITIVITY = 0.07;
    clientGame->yaw += deltaX * SENSITIVITY;
    clientGame->pitch += deltaY * SENSITIVITY;
    clientGame->pitch = glm::clamp(clientGame->pitch, -89.0f, 89.0f);
    std::printf("camera yaw(%f) pitch(%f)\n\n", clientGame->yaw, clientGame->pitch); // in degrees (human readable)
    
    // (todo) Graphics: update camera's forward vector based on new orientation. 

}



// just a lonely helper method now. 
// (todo) client should compute camera angle (for local render) and send the vec3 to server (for shooting ray)
void calculateCameraDirection(unsigned int client_id, float yaw, float pitch) {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    
}
