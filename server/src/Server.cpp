﻿// server.cpp : Defines the entry point for the application.
//

#include "Server.h"
#include <thread>

void serverLoop();
std::unique_ptr<ServerGame> server;

int main()
{
	std::cout << "Hello, I'm the server." << std::endl;

    // initialize the server
    server = std::make_unique<ServerGame>();
    serverLoop();

	return 0;
}

void serverLoop()
{
    auto time=std::chrono::system_clock::now();
    // declare a 33ms duration
    std::chrono::milliseconds tickLenMs{33};
    std::chrono::duration<double, std::milli> tickLen(tickLenMs);
    // int counter = 0;
    // auto nextTick=time+tickLen;
    while (true)
    {
        /* if (counter % 30 == 0) {
            std::cout << counter / 30 << std::endl;
        } */
        // Start timer
        auto start = std::chrono::high_resolution_clock::now();

        // Do updates
        server->update();

        // Sleep for the remaining time
        auto remainingTime = tickLen - (std::chrono::high_resolution_clock::now() - start);
<<<<<<< HEAD
        // std::cout << "Remaining time: " << std::chrono::duration_cast<std::chrono::milliseconds>(remainingTime).count() << " ms" << std::endl;  // uncomment to see idle time per tick (was 32ms)
=======
        // std::cout << "Remaining time: " << std::chrono::duration_cast<std::chrono::milliseconds>(remainingTime).count() << " ms" << std::endl;
>>>>>>> main
        if (remainingTime > std::chrono::duration<double>::zero()) {
            std::this_thread::sleep_for(remainingTime);
        } else {
            // not good
        }
        // counter++;
        // nextTick+=tickLen;
    }
}

