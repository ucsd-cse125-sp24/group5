// client.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include "ClientGame.h"

void clientLoop(void);
void init();
void close();
extern std::unique_ptr<ClientGame> client;