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
    // TODO: change this line to enable fullscreen
    window = glfwCreateWindow(800, 600, "Vivaldi", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "GLFW failed to create window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    sge::ModelComposite m("./model/env.obj");
    std::cout << "made it!\n";
}

/**
 * Callback function for GLFW when user resizes window
 * @param window
 * @param width
 * @param height
 */
void sge::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

/**
 * Shitty graphics engine loop callback
 */
void sge::sgeLoop()
{
    // Process events
    glfwPollEvents();

    // Render
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red background
    glClear(GL_COLOR_BUFFER_BIT);

    // Swap buffers
    glfwSwapBuffers(sge::window);
}