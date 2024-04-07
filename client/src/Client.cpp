// client.cpp : Defines the entry point for the application.
//

#include "Client.h"

void clientLoop(void);
ClientGame* client;


int main()
{
	std::cout << "Hello, I'm the client." << std::endl;


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

//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
//#include <iostream>
//
//// Function to handle resizing of the window
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//    glViewport(0, 0, width, height);
//}
//
//int main()
//{
//    glm::mat4 m;
//    // Initialize GLFW
//    std::cout << "sup adsfa;lsdkjfaskdl;fj\n";
//    if (!glfwInit())
//    {
//        std::cerr << "Failed to initialize GLFW" << std::endl;
//        return -1;
//    }
//
//    // Create a GLFW window
//    GLFWwindow* window = glfwCreateWindow(800, 600, "GLFW/GLEW Test", nullptr, nullptr);
//    if (!window)
//    {
//        std::cerr << "Failed to create GLFW window" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//
//    // Make the window's context current
//    glfwMakeContextCurrent(window);
//
//    // Initialize GLEW
//    if (glewInit() != GLEW_OK)
//    {
//        std::cerr << "Failed to initialize GLEW" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//
//    // Set the viewport size
//    int width, height;
//    glfwGetFramebufferSize(window, &width, &height);
//    glViewport(0, 0, width, height);
//
//    // Register callback for window resizing
//    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//
//    // Main loop
//    while (!glfwWindowShouldClose(window))
//    {
//        // Process events
//        glfwPollEvents();
//
//        // Render
//        glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red background
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        // Swap buffers
//        glfwSwapBuffers(window);
//    }
//
//    // Terminate GLFW
//    glfwTerminate();
//    return 0;
//}
