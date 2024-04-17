// client.cpp : Defines the entry point for the application.
//
#include <chrono>
#include <thread>
#include "Client.h"

std::unique_ptr<ClientGame> clientGame;
sge::ModelComposite *ptr; // TODO: Delete this later, here for debugging graphics engine
sge::ModelComposite *anchor1Ptr;
sge::ModelComposite *anchor2Ptr;
double lastX, lastY;    // last cursor position
bool enableInput = false;

int main()
{
    std::cout << "Hello, I'm the client." << std::endl;

    sge::sgeInit();
    // comment out ModelComposite stuff if you're debugging networking
    sge::ModelComposite object("rock_w_tex/rock2.obj"); // this is here for testing purposes (for now)
    ptr = &object;
    sge::ModelComposite anchor1("rock_w_tex/rock2.obj");
    anchor1Ptr = &anchor1;
    sge::ModelComposite anchor2("rock_w_tex/rock2.obj");
    anchor2Ptr = &anchor2;

    clientGame = std::make_unique<ClientGame>();

    glfwSetFramebufferSizeCallback(sge::window, framebufferSizeCallback);
    // Register keyboard input callbacks
    glfwSetKeyCallback(sge::window, key_callback);
    // Register cursor input callbacks
    glfwSetMouseButtonCallback(sge::window, mouse_button_callback);
    glfwSetInputMode(sge::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  // virtual & unlimited cursor movement for camera control , will hide cursor!
    glfwGetCursorPos(sge::window, &lastX, &lastY);     // init
    glfwSetCursorPosCallback(sge::window, cursor_callback);

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
        // Poll for and process events (e.g. keyboard & mouse input callbacks)
        glfwPollEvents();

        // Send these input to server
        clientGame->sendClientInputToServer();

        // Receive updates from server
        clientGame->network->receiveUpdates();

        // Update local game state

        // Render
        sge::ModelComposite::updateCameraToFollowPlayer(clientGame->positions[clientGame->client_id], clientGame->yaw, clientGame->pitch);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        anchor1Ptr->render(glm::vec3(-15.0f, 0.0f, -9.0f), 0);
        anchor2Ptr->render(glm::vec3(15.0f, 10.0f, -9.0f), 0);
        ptr->render(clientGame->positions[clientGame->client_id], clientGame->yaw);

        // Swap buffers
        glfwSwapBuffers(sge::window);
    }

    // Terminate GLFW
    glfwTerminate();
    std::cout << "Good bye --- client terminal.\n";
    return;
}

/**
 * Callback function for GLFW when user resizes window
 * @param window GLFW window object
 * @param width Desired window width
 * @param height Desired window height
 */
void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (!enableInput) return;
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
        case GLFW_KEY_ESCAPE:
            enableInput = false;
            glfwSetInputMode(sge::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
        case GLFW_KEY_ESCAPE:
//            glfwSetInputMode(sge::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            break;
        default:
            std::cout << "unrecognized key release, gg\n";
            break;
        }
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (!enableInput && button == GLFW_MOUSE_BUTTON_LEFT) {
        enableInput = true;
        glfwSetInputMode(sge::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!enableInput) return;
    double deltaX = xpos - lastX;
    double deltaY = lastY - ypos;  // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;
    std::printf("cursor moved right(%lf) up(%lf)\n", deltaX, deltaY);
    
    const double SENSITIVITY = 0.07;
    clientGame->yaw += deltaX * SENSITIVITY;
    clientGame->pitch += deltaY * SENSITIVITY;
    clientGame->pitch = glm::clamp(clientGame->pitch, -89.0f, 89.0f);
    std::printf("cursor yaw(%f) pitch(%f)\n\n", clientGame->yaw, clientGame->pitch); // in degrees (human readable)
    
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
