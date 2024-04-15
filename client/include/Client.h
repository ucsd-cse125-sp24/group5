// client.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include "sge/ShittyGraphicsEngine.h"
#include "ClientGame.h"

void clientLoop(void);
void sleep(int ms);

extern std::unique_ptr<ClientGame> clientGame;

class Client {
    public:
        // Keyboard input stuffs
        void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};