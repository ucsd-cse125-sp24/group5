// server.cpp : Defines the entry point for the application.
//

#include "Server.h"

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
    // declare a 50ms duration
    std::chrono::milliseconds tickLenMs{33};
    std::chrono::duration<double, std::milli> tickLen(tickLenMs);
    auto nextTick=time+tickLen;
    while (true)
    {
        // Start timer
        server->update();
        while(time<nextTick) {
            // could do sleep if spinning is bad
            time=std::chrono::system_clock::now();
        }
        nextTick+=tickLen;
    }
}

