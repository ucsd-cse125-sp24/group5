//
// Created by benjx on 4/8/2024.
//
#include "sge/ShittyGraphicsEngine.h"

GLFWwindow *sge::window;
int sge::windowHeight, sge::windowWidth;

/**
 * Initialize shitty graphics engine, does not set up GLFW callback functions
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
    glEnable(GL_DEPTH_TEST);

    sge::initShaders();
    perspectiveMat = glm::perspective(glm::radians(90.0f), (float)sge::windowWidth / (float)sge::windowHeight, 0.5f, 1000.0f);
    glUniformMatrix4fv(sge::perspectivePos, 1, GL_FALSE, &perspectiveMat[0][0]);
}

/**
 * Load models from disk into GraphicsGeometry.h's models vector
 */
void sge::loadModels() {
    // Avoid potentially needing to copy models to a new underlying vector, modelcomposites are fat, so much memory usage :(
    models.reserve(NUM_MODELS);
    std::string pathPrefix = "./models/";
    // NOTE: ENSURE THAT FILEPATHS FOLLOWS THE SAME ORDERING AS MODELINDEX ENUM IN GRAPHICSGEOMETRY.H
    std::string filePaths[NUM_MODELS] =
            {
            "map_1_test.obj",
            "rock_w_tex/rock2.obj"
            };
    for (unsigned int i = 0; i < NUM_MODELS; i++) {
        models.push_back(std::make_unique<ModelComposite>(pathPrefix + filePaths[i]));
    }
}
