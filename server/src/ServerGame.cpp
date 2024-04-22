#include "ServerGame.h"

unsigned int ServerGame::client_id;

ServerGame::ServerGame(void)
{
    // id's to assign clients for our table
    client_id = 0;

    // set up the server network to listen
    network = std::make_unique<ServerNetwork>(this);

    // Initialize game world
    std::cout << "Initializing server game world...\n";
    for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
        positions[i] = glm::vec3(i*10.0f, 0.0f, -(i%2)*8.0f);
        velocities[i] = glm::vec3(0.0f, 0.0f, 0.0f);
        doubleJumpUsed[i] = 0;
        doubleJumpCD[i] = 0;
    }
    world.init();
    world.printDebug();
}

void ServerGame::update()
{
    // get new clients
    if (network->acceptNewClient(client_id))
    {
        std::printf("client %d has been connected to the server\n", client_id);

        client_id++;
    }

    network->receiveFromClients();
    // do game logic here and then do all the sends back
    world.updateAllSystems();
}

void ServerGame::handleInitConnection(unsigned int client_id) {
    std::cout << "Server received init packet from client " << client_id << std::endl;

    // This is a new client, so tell it what its id is
    IssueIdentifierUpdate update;
    update.client_id = client_id;
    network->sendIssueIdentifierUpdate(update);
}


void ServerGame::handleClientActionInput(unsigned int client_id, ClientToServerPacket& packet)
{
    // for testing now 
    // std::printf("client(%d): W(%d) A(%d) S(%d) D(%d) Jump(%d) | yaw(%f) pitch(%f)\n", client_id, packet.requestForward, packet.requestLeftward, packet.requestBackward, packet.requestRightward, packet.requestJump, packet.yaw, packet.pitch);

    // Book keeping for each client (so they can render each other)
    yaws[client_id] = packet.yaw;
    pitches[client_id] = packet.pitch;

    // Update player's moving direction (no pitch here, cuz you can't fly)
    glm::vec3 forward_direction;
    forward_direction.x = cos(glm::radians(packet.yaw));
    forward_direction.y = 0;
    forward_direction.z = sin(glm::radians(packet.yaw));
    forward_direction = glm::normalize(forward_direction);

    glm::vec3 rightward_direction = glm::normalize(glm::cross(forward_direction, glm::vec3(0,1,0)));

    glm::vec3 total_direction = glm::vec3(0);

    float air_modifier = (positions[client_id].y <= 0.0f)?1:0.6;

    if (packet.requestForward)      total_direction += forward_direction;
    if (packet.requestBackward)     total_direction -= forward_direction;
    if (packet.requestLeftward)     total_direction -= rightward_direction;
    if (packet.requestRightward)    total_direction += rightward_direction;

    if(total_direction!=glm::vec3(0)) total_direction = glm::normalize(total_direction);

    velocities[client_id] += total_direction * MOVEMENT_SPEED * air_modifier;

    if (packet.requestForward || packet.requestBackward || packet.requestLeftward || packet.requestRightward) {
        std::printf("client(%d) at position x(%f) y(%f) z(%f)\n", client_id, positions[client_id].x, positions[client_id].y, positions[client_id].z);
        // Maybe have this function return the true new positions, and overwrite the packet data?
        // world.movePlayer(client_id, positions[client_id].x, positions[client_id].y, positions[client_id].z);
        // world.printDebug();
    }


    if (positions[client_id].y <= 0.0f) {
        velocities[client_id].x*=GROUND_FRICTION;
        velocities[client_id].z*=GROUND_FRICTION;
    } else {
        velocities[client_id].x*=AIR_FRICTION;
        velocities[client_id].z*=AIR_FRICTION;
    }
    // Update velocity with accelerations (gravity, player jumping, etc.)
    velocities[client_id].y -= jumpHeld[client_id]?GRAVITY:GRAVITY*FASTFALL_INCREASE;

    if(jumpHeld[client_id]&&!packet.requestJump) {
        jumpHeld[client_id]=false;
    }

    if (!jumpHeld[client_id] && packet.requestJump && doubleJumpUsed[client_id] < MAX_JUMPS_ALLOWED) {
        doubleJumpCD[client_id] = DOUBLE_JUMP_COOLDOWN_TICKS;
        doubleJumpUsed[client_id]++;
        velocities[client_id].y = JUMP_SPEED;     // as god of physics, i endorse = and not += here
        jumpHeld[client_id]=true;
    }
    std::printf("Jumped! (double jumps used: %d) (CD:%d)\n", doubleJumpUsed[client_id], doubleJumpCD[client_id]);

    // Use velocity to further change the player's position 
    positions[client_id] += velocities[client_id];


    // Simple physics: don't fall below the map (assume y=0 now; will change once we have map elevation data / collision boxes)
    if (positions[client_id].y <= 0.0f) {
        // reset jump states
        positions[client_id].y = 0.0f;
        velocities[client_id].y = 0.0f;
        doubleJumpUsed[client_id] = 0;
        doubleJumpCD[client_id] = 0;
    }
    
    
    
    // For now assume map is flat and player stays on the ground (y=0). 
    // *To deal with ups and downs due to unflat map, add more logic to bump player up (after calculating their new position). 


}

ServerGame::~ServerGame(void) {

}
