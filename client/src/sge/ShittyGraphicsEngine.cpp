//
// Created by benjx on 4/8/2024.
//
#include "sge/ShittyGraphicsEngine.h"

GLFWwindow *sge::window;
int sge::windowHeight, sge::windowWidth;

/**
 * Initialize shitty graphics engine
 */
void sge::sgeInit()
{
    // Initialize GLFW
    if (!glfwInit()) {
        std::cout << "GLFW failed to start\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // TODO: change this line to enable fullscreen
    window = glfwCreateWindow(800, 600, "Vivaldi", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "GLFW failed to create window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    // Initialize GLEW
    #ifndef __APPLE__
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    #endif

    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    // Register keyboard input callbacks
    glfwSetKeyCallback(window, keyCallback);
    glEnable(GL_DEPTH_TEST);

    sge::initShaders();
}

/**
 * Callback function for GLFW when user resizes window
 * @param window GLFW window object
 * @param width Desired window width
 * @param height Desired window height
 */
void sge::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

/**
 * Callback function for Keypresses and such
 */
void sge::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
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
