﻿// client.cpp : Defines the entry point for the application.
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

    // I move the setup for glfw to after the lobby screen are done
    // 
    // 
    //glfwSetFramebufferSizeCallback(sge::window, framebufferSizeCallback);
    //// Register keyboard input callbacks
    //glfwSetKeyCallback(sge::window, key_callback);
    //// Register cursor input callbacks
    //glfwSetMouseButtonCallback(sge::window, mouse_button_callback);
    //glfwSetInputMode(sge::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  // virtual & unlimited cursor movement for camera control , will hide cursor!
    //glfwGetCursorPos(sge::window, &lastX, &lastY);     // init
    //glfwSetCursorPosCallback(sge::window, cursor_callback);




    sound::initSoundManager();
    ui::initUIManager();
    

    

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

/**
 * Main client game loop
 */
void clientLoop()
{
    ///////////// Graphics set up stuffs above^ /////////////
    int i = 0;

    
    // Main loop
    while (!glfwWindowShouldClose(sge::window))
    {
        // Poll for and process events (e.g. keyboard & mouse input callbacks)
        glfwPollEvents();

        // when the lobby screen are done, transition to the game
        if (ui::isTransitioningToGame) {
            
            sge::secondStageInit();

            glfwSetFramebufferSizeCallback(sge::window, framebufferSizeCallback);
            // Register keyboard input callbacks
            glfwSetKeyCallback(sge::window, key_callback);
            // Register cursor input callbacks
            glfwSetMouseButtonCallback(sge::window, mouse_button_callback);
            glfwSetInputMode(sge::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  // virtual & unlimited cursor movement for camera control , will hide cursor!
            glfwGetCursorPos(sge::window, &lastX, &lastY);     // init
            glfwSetCursorPosCallback(sge::window, cursor_callback);


            ui::isTransitioningToGame = false;
        }


        if (ui::isInLobby) {
            ui::uiManager->lobby();

            // if the player makes selection, send the selection to server
            if (ui::uiManager->selectedCharacterUID != NO_CHARACTER) {
                if (!ui::uiManager->isLobbySelectionSent) {
                    clientGame->sendLobbySelectionToServer(ui::uiManager->selectedCharacterUID);
                    ui::uiManager->isLobbySelectionSent = true;
                }
            }
            // the current character on select (not confirmed) --> play its theme song 
            if (ui::uiManager->charJustChanged()) {
                int prevID = ui::uiManager->getPrevCharSelection();
                sound::soundManager->stopCharacterTheme(prevID);
                int currID = ui::uiManager->getCurrentCharSelection();
                sound::soundManager->playCharacterTheme(currID);

            }

            // receive update from server - here we only interest in the lobby selection
            clientGame->network->receiveUpdates();

        }
        else {
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
                                            clientGame->cameraDistances[clientGame->client_id]
                                            );

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
            
            // Render framebuffer with postprocessing
            glDisable(GL_CULL_FACE);
            sge::screenProgram.useShader();
            sge::postprocessor.drawToScreen();

            // Draw UI
            // printf("current season = %d\n", clientGame->gameSeason);

            sge::lineUIShaderProgram.drawCrossHair(clientGame->shootingEmo); // let clientGame decide the emotive scale
            clientGame->updateShootingEmo();

            // Swap buffers
            glfwSwapBuffers(sge::window);

            if (i % 1000 == 0) {
                // I no like this >:( - ben
    //            sound::soundManager->explosionSound();
            }

            i++;
        }

        
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
