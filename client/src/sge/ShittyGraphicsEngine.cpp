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
    window = glfwCreateWindow(1400, 800, "Vivaldi", nullptr, nullptr);
    // window = glfwCreateWindow(800, 600, "Vivaldi", glfwGetPrimaryMonitor(), nullptr);  // full screen mode
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

    std::printf("window fucking size %d, %d\n", sge::windowWidth, sge::windowHeight);
    glfwGetFramebufferSize(window, &sge::windowWidth, &sge::windowHeight);
    std::printf("window fucking size %d, %d\n", sge::windowWidth, sge::windowHeight);  // why is it 2x on mac?

    glViewport(0, 0, sge::windowWidth, sge::windowHeight);
    glEnable(GL_DEPTH_TEST);   // Only render stuff closest to camera
    glEnable(GL_STENCIL_TEST); // TODO: is to allow for rendering outlines around objects later. (e.g. outline around egg or something)
    glEnable(GL_CULL_FACE);

    sge::initShaders();

    // Set default camera perspective projection matrix
    perspectiveMat = glm::perspective(glm::radians(90.0f), (float)sge::windowWidth / (float)sge::windowHeight, 0.5f, 1000.0f);
    particleProgram.useShader();
    particleProgram.updatePerspectiveMat(perspectiveMat);
    defaultProgram.useShader();
    defaultProgram.updatePerspectiveMat(perspectiveMat);
    lineShaderProgram.useShader();
    lineShaderProgram.updatePerspectiveMat(perspectiveMat);

    generator.seed(std::random_device()()); // Seed random number generator used by particle system
}

/**
 * Gracefully end shitty graphics engine
 */
void sge::sgeClose() {
    models.clear();
    postprocessor.deletePostprocessor();
    shadowprocessor.deleteShadowmap();
    lineShaderProgram.deleteLineShader();
    crosshairShaderProgram.deleteLineUI();
    deleteTextures();
    glfwTerminate();
}

/**
 * Load models from disk into GraphicsGeometry.h's models vector
 */
void sge::loadModels() {
    // Avoid potentially needing to copy models to a new underlying vector, modelcomposites are fat, so much memory usage :(
    models.reserve(NUM_MODELS);
    std::string pathPrefix = (std::string)(PROJECT_PATH) + "/client/models/";
    
    // NOTE: ENSURE THAT FILEPATHS FOLLOWS THE SAME ORDERING AS MODELINDEX ENUM IN GRAPHICSGEOMETRY.H
    // Modify ModelIndex enum to add more models
    std::string filePaths[NUM_MODELS] =
            {
            SetupParser::getValue("map-path"),
            "bear_centered.glb",
            "fox3.glb",
            "egg.obj",
            "char_temp.obj",
            "egg.obj",
            "egg.obj",
            "egg.obj"
            };
    for (unsigned int i = 0; i < NUM_MODELS; i++) {
        models.push_back(std::make_unique<ModelComposite>(pathPrefix + filePaths[i]));
    }
    // manually set the bear to use animation 0, tick 500 when not moving
    models[BEAR]->setStillAnimation(0, 500);
}
