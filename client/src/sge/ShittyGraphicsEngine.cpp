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
    glEnable(GL_DEPTH_TEST);   // Only render stuff closest to camera
//    glEnable(GL_STENCIL_TEST); // TODO: is to allow for rendering outlines around objects later. (e.g. outline around egg or something)
//    glEnable(GL_CULL_FACE);

    sge::initShaders();

    // Set default camera perspective projection matrix
    defaultProgram.useShader();
    perspectiveMat = glm::perspective(glm::radians(90.0f), (float)sge::windowWidth / (float)sge::windowHeight, 0.5f, 1000.0f);
    defaultProgram.updatePerspectiveMat(perspectiveMat);
}

/**
 * Gracefully end shitty graphics engine
 */
void sge::sgeClose() {
    models.clear();
    postprocessor.deletePostprocessor();
    shadowprocessor.deleteShadowmap();
    deleteTextures();
    glfwTerminate();
}

/**
 * Load models from disk into GraphicsGeometry.h's models vector
 */
void sge::loadModels() {
    // Avoid potentially needing to copy models to a new underlying vector, modelcomposites are fat, so much memory usage :(
    models.reserve(NUM_MODELS);
    std::string pathPrefix = "./models/";
    // NOTE: ENSURE THAT FILEPATHS FOLLOWS THE SAME ORDERING AS MODELINDEX ENUM IN GRAPHICSGEOMETRY.H
    // Modify ModelIndex enum to add more models
    std::string filePaths[NUM_MODELS] =
            {
            "map_2_test/map_2_test.obj",
            "char_temp.obj",
            "bear_centered.glb",
            "egg.obj"
            };
    for (unsigned int i = 0; i < NUM_MODELS; i++) {
        models.push_back(std::make_unique<ModelComposite>(pathPrefix + filePaths[i]));
    }
    // manually set the bear to use animation 0, tick 500 when not moving
    models[BEAR]->setStillAnimation(0, 500);
}
