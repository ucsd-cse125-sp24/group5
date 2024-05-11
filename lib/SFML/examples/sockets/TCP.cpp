////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network.hpp>

#include <iomanip>
#include <iostream>
#include <optional>

#include <cstddef>


////////////////////////////////////////////////////////////
/// Launch a server, wait for an incoming connection,
/// send a message and wait for the answer.
///
////////////////////////////////////////////////////////////
void runTcpServer(unsigned short port)
{
    // Create a server socket to accept new connections
    sf::TcpListener listener;

    // Listen to the given port for incoming connections
    if (listener.listen(port) != sf::Socket::Status::Done)
        return;
    std::cout << "Server is listening to port " << port << ", waiting for connections... " << std::endl;

    // Wait for a connection
    sf::TcpSocket socket;
    if (listener.accept(socket) != sf::Socket::Status::Done)
        return;
    std::cout << "Client connected: " << socket.getRemoteAddress().value() << std::endl;

    // Send a message to the connected client
    const char out[] = "Hi, I'm the server";
    if (socket.send(out, sizeof(out)) != sf::Socket::Status::Done)
        return;
    std::cout << "Message sent to the client: " << std::quoted(out) << std::endl;

    // Receive a message back from the client
    char        in[128];
    std::size_t received = 0;
    if (socket.receive(in, sizeof(in), received) != sf::Socket::Status::Done)
        return;
    std::cout << "Answer received from the client: " << std::quoted(in) << std::endl;
}


////////////////////////////////////////////////////////////
/// Create a client, connect it to a server, display the
/// welcome message and send an answer.
///
////////////////////////////////////////////////////////////
void runTcpClient(unsigned short port)
{
    // Ask for the server address
    std::optional<sf::IpAddress> server;
    do
    {
        std::cout << "Type the address or name of the server to connect to: ";
        std::cin >> server;
    } while (!server.has_value());

    // Create a socket for communicating with the server
    sf::TcpSocket socket;

    // Connect to the server
    if (socket.connect(server.value(), port) != sf::Socket::Status::Done)
        return;
    std::cout << "Connected to server " << server.value() << std::endl;

    // Receive a message from the server
    char        in[128];
    std::size_t received = 0;
    if (socket.receive(in, sizeof(in), received) != sf::Socket::Status::Done)
        return;
    std::cout << "Message received from the server: " << std::quoted(in) << std::endl;

    // Send an answer to the server
    const char out[] = "Hi, I'm a client";
    if (socket.send(out, sizeof(out)) != sf::Socket::Status::Done)
        return;
    std::cout << "Message sent to the server: " << std::quoted(out) << std::endl;
}
