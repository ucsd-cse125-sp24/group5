// client.cpp : Defines the entry point for the application.
//
#include <chrono>
#include <thread>
#include "Client.h"


std::unique_ptr<ClientGame> clientGame;
std::vector<std::shared_ptr<sge::ModelEntityState>> entities;
std::vector<std::shared_ptr<sge::DynamicModelEntityState>> movementEntities;
std::unique_ptr<sge::ParticleEmitterEntity> emitter;
double lastX, lastY;    // last cursor position
bool enableInput = false;


int main()
{
    std::cout << "Hello, I'm the client." << std::endl;

    // Initialize graphics engine
    sge::sgeInit();

    // Load 2d images for UI
    sge::loadUIs();

    // Load 3d models for graphics engine
    sge::loadModels();

    sge::emitters.push_back(std::make_unique<sge::ParticleEmitter>());

    clientGame = std::make_unique<ClientGame>();

    // Create permanent graphics engine entities
    entities.push_back(std::make_shared<sge::ModelEntityState>(MAP, glm::vec3(0.0f, 0.0f, 0.0f))); // with no collision (yet), this prevents player from falling under the map.
    for (unsigned int i = 0; i < 4; i++) { // Player graphics entities
        std::shared_ptr<sge::DynamicModelEntityState> playerEntity = std::make_shared<sge::DynamicModelEntityState>(FOX, movementEntities.size());

        entities.push_back(playerEntity);
        clientGame->playerIndices.push_back(movementEntities.size());
        movementEntities.push_back(playerEntity);
    }
    std::shared_ptr<sge::DynamicModelEntityState> egg = std::make_shared<sge::DynamicModelEntityState>(EGG, movementEntities.size());
    entities.push_back(egg);
    movementEntities.push_back(egg);
    for (unsigned int i = 0; i < NUM_PROJ_TYPES; i++) {
        for (unsigned int j = 0; j < NUM_EACH_PROJECTILE; j++) {
            std::shared_ptr<sge::DynamicModelEntityState> projEntity = std::make_shared<sge::DynamicModelEntityState>(SUMMER_BALL, movementEntities.size());
            entities.push_back(projEntity);
            movementEntities.push_back(projEntity);
        }
    }

    glfwSetFramebufferSizeCallback(sge::window, framebufferSizeCallback);
    // Register keyboard input callbacks
    glfwSetKeyCallback(sge::window, key_callback);
    // Register cursor input callbacks
    glfwSetMouseButtonCallback(sge::window, mouse_button_callback);
    glfwSetInputMode(sge::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  // virtual & unlimited cursor movement for camera control , will hide cursor!
    glfwGetCursorPos(sge::window, &lastX, &lastY);     // init
    glfwSetCursorPosCallback(sge::window, cursor_callback);

    sound::initSoundManager();
    emitter = std::make_unique<sge::DiskParticleEmitterEntity>(2,
                                                           0.5f,
                                                           0.0f,
                                                           1000,
                                                           std::vector<float>({0.5f, 0.5f}),
                                                           std::vector<glm::vec4>({glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 1, 1)}),
                                                           std::vector<glm::vec4>({glm::vec4(1, 1, 0, 0), glm::vec4(0, 1, 0, 0)}),
                                                           glm::vec3(0.0f, 0.0f, 0.0f),
                                                           glm::vec3(-0.5f, 0.5f, -0.5f),
                                                           10.0f,
                                                           -0.5f,
                                                           glm::vec3(0.0f, -0.00f, 0.0f),
                                                           clientGame->client_id,
                                                           glm::vec3(0.0f, 2.0f, 0.0f), 3.0f);
    emitter->setActive(true);
    clientLoop();
    sge::sgeClose();
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

void updateSunPostion(glm::vec3 &sunPos, int t) {
    // parameters for the parabolic path
    float a = 0.01; // adjust the curvature of the parabola
    float b = 0.1;   // linear term coefficient
    float c = 100.0;  // constant term, adjusting the initial height

    // Calculate the x position based on time
    sunPos.z = 100.0 * cos(t / 190.0); 

    // Calculate the z position based on the parabolic equation
    sunPos.x = a * sunPos.z * sunPos.z + b * sunPos.z + c;
    sunPos.x -= 20.0f;

    // Update the y position to simulate vertical movement
    sunPos.y = 120.0 + 5.0 * sin(t / 190.0);

}

/**
 * Main client game loop
 */
void clientLoop()
{
    ///////////// Graphics set up stuffs above^ /////////////
    int i = 0;

    // Update shadow map with current state of entities/poses
    // TODO: Avoid hard coding this
    // If we want dynamic global lighting (i.e. change time of day), change the light vector stuff
    // Projection matrix for light, use orthographic for directional light, perspective for point light
    glm::mat4 lightProjection = glm::ortho(-40.0, 40.0, -40.0, 40.0, -40.0, 40.0);
    // Light position, also used as light direction for directional lights
    glm::vec3 lightPos(5, 5, 0);
    // Where light is "pointing" towards
    glm::vec3 lightCenter(0, 0, 0);
    // This exists because lookAt wants an up vector, not totally necessary tho
    glm::vec3 lightUp(0, 1, 0);
    // Light viewing matrix
    glm::mat4 lightView = glm::lookAt(lightPos, lightCenter, lightUp);
    
    // Main loop
    while (!glfwWindowShouldClose(sge::window))
    {
        // Poll for and process events (e.g. keyboard & mouse input callbacks)
        glfwPollEvents();

        // Send these input to server
        clientGame->sendClientInputToServer();

        // Receive updates from server/update local game state
        clientGame->network->receiveUpdates();

        for (unsigned int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
            movementEntities[i]->setAnimation(clientGame->animations[i]);
        }

        // Render all entities that use the default shaders to the gBuffer
        for (unsigned int i = 0; i < entities.size(); i++) {
            entities[i]->update();
        }

        // // Update shadow map with current state of entities/poses
        // // TODO: Avoid hard coding this
        // // If we want dynamic global lighting (i.e. change time of day), change the light vector stuff
        // // Projection matrix for light, use orthographic for directional light, perspective for point light
        // glm::mat4 lightProjection = glm::ortho(-40.0, 40.0, -40.0, 40.0, -40.0, 40.0);
        // // Light position, also used as light direction for directional lights
        // glm::vec3 lightPos(5, 5, 0);
        // // Where light is "pointing" towards
        // glm::vec3 lightCenter(0, 0, 0);
        // // This exists because lookAt wants an up vector, not totally necessary tho
        // glm::vec3 lightUp(0, 1, 0);
        // // Light viewing matrix
        // glm::mat4 lightView = glm::lookAt(lightPos, lightCenter, lightUp);

        updateSunPostion(lightPos, i);

        // Give shaders global lighting information
        // This only works with 1 shadow-casting light source at the moment
        sge::shadowProgram.updatePerspectiveMat(lightProjection);
        sge::shadowProgram.updateViewMat(lightView);

        sge::defaultProgram.updateLightPerspectiveMat(lightProjection);
        sge::defaultProgram.updateLightViewMat(lightView);
        sge::defaultProgram.updateLightDir(glm::vec4(lightPos, 0));

        sge::shadowProgram.useShader();
        // If we want multiple shadow maps, we'll need to draw EVERYTHING to each one
        sge::shadowprocessor.drawToShadowmap();
        for (unsigned int i = 0; i < entities.size(); i++) {
            entities[i]->drawShadow();
        }
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        sge::defaultProgram.useShader();
        sge::updateCameraToFollowPlayer(clientGame->positions[clientGame->client_id],
                                        clientGame->yaws[clientGame->client_id],
                                        clientGame->pitches[clientGame->client_id],
                                        clientGame->cameraDistances[clientGame->client_id],
                                        clientGame->gameOver);

        // Draw everything to framebuffer (gbuffer)
        sge::postprocessor.drawToFramebuffer();

        // Uncomment the below to display wireframes
//        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

        // Pass updated shadow map to toon shader
        sge::shadowprocessor.updateShadowmap();
        // Render all entities that use the default shaders to the gBuffer
        for (unsigned int i = 0; i < entities.size(); i++) {
            entities[i]->draw();
        }

        // Draw particles
        // Only enable alpha blending for color attachment 0 (the one holding fragment colors)
        glEnablei(GL_BLEND, 0);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        sge::particleProgram.useShader();
        emitter->update();
        if (i > 1000 && i % 100 == 0) {
            emitter->explode();
        }
        emitter->draw();
        glDisablei(GL_BLEND, 0);


        // Render ephemeral entities (bullet trail, fireballs, etc.)
        sge::lineShaderProgram.useShader();
        for (BulletToRender& b : clientGame->bulletQueue) {
            sge::lineShaderProgram.renderBulletTrail(b.start, b.currEnd);
        }
        clientGame->updateBulletQueue();
        // TESTING: show xyz axis as bullet trails
        sge::lineShaderProgram.renderBulletTrail(glm::vec3(0), glm::vec3(50,0,0));
        sge::lineShaderProgram.renderBulletTrail(glm::vec3(0), glm::vec3(0,50,0));
        sge::lineShaderProgram.renderBulletTrail(glm::vec3(0,5,0), glm::vec3(0,5,50));

        // render tags above other players
        for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
            if (i == clientGame->client_id) continue;
            sge::billboardProgram.renderPlayerTag(clientGame->positions[i], sge::UIs[5]->texture);
        }

        // Render framebuffer with postprocessing
        glDisable(GL_CULL_FACE);
        sge::screenProgram.useShader();
        sge::postprocessor.drawToScreen();

        // TESTING moving sun: put a billboard quad at sun's location
        glEnable(GL_BLEND); // enable alpha blending for images with transparent background
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        sge::billboardProgram.renderPlayerTag(lightPos- glm::vec3(0,1.3,0), sge::UIs[2]->texture, 20);
        glDisable(GL_BLEND);
        // Draw crosshair
        sge::crosshairShaderProgram.drawCrossHair(clientGame->shootingEmo); // let clientGame decide the emotive scale
        clientGame->updateShootingEmo();
        
        // Render UIs
        sge::renderAllUIs(clientGame->currentSeason);
        sge::renderAllTexts(clientGame->healths[clientGame->client_id],
                            clientGame->scores[0] + clientGame->scores[1],
                            clientGame->scores[2] + clientGame->scores[3],
                            clientGame->currentSeason,
                            enableInput,
                            clientGame->gameOver,
                            clientGame->winner
                            );


        // Swap buffers
        glfwSwapBuffers(sge::window);

        if (i % 1000 == 0) {
            // I no like this >:( - ben
//            sound::soundManager->explosionSound();
        }

        i++;
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
    sge::windowWidth = width;
    sge::windowHeight = height;
    sge::postprocessor.resizeFBO();
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
            sound::soundManager->jumpSound();
            break;
        case GLFW_KEY_E:
            clientGame->requestThrowEgg = true;
            break;
        case GLFW_KEY_M:
            sound::soundManager->muteBgmToggle();
            break;
        case GLFW_KEY_ESCAPE:
            enableInput = false;
            glfwSetInputMode(sge::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case GLFW_KEY_UP:
            // move up ui box? todo: for positioning ui entities
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
        case GLFW_KEY_E:
            clientGame->requestThrowEgg = false;
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
    if (enableInput) {
        if (action == GLFW_PRESS) {
            switch (button)
            {
            case GLFW_MOUSE_BUTTON_LEFT:
                clientGame->requestShoot = true;
                // sound::soundManager->shootingSound();
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                clientGame->requestAbility = true;
                break;
            }
        }
        else if (action == GLFW_RELEASE) {
            switch (button)
            {
            case GLFW_MOUSE_BUTTON_LEFT:
                clientGame->requestShoot = false;
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                clientGame->requestAbility = false;
                break;
            }
        }
    }
    else if (!enableInput && button == GLFW_MOUSE_BUTTON_LEFT) {
        enableInput = true;
        glfwGetCursorPos(sge::window, &lastX, &lastY);  // prevent glitching
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
    // std::printf("cursor moved right(%lf) up(%lf)\n", deltaX, deltaY);

    const double SENSITIVITY = 0.07;
    clientGame->playerYaw += deltaX * SENSITIVITY;
    clientGame->playerPitch += deltaY * SENSITIVITY;
    clientGame->playerPitch = glm::clamp(clientGame->playerPitch, -89.0f, 89.0f);
    // std::printf("cursor yaw(%f) pitch(%f)\n\n", clientGame->playerYaw, clientGame->playerPitch); // in degrees (human readable)
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
