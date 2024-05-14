#include "ClientGame.h"

ClientGame::ClientGame()
{
    network = std::make_unique<ClientNetwork>(this);
    client_id = 0;
    std::cout << "Initializing client game world...\n";
    for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
        positions[i] = glm::vec3(i*10.0f, 0.0f, -(i%2)*8.0f);
        yaws[i] = -90.0f;
        cameraDistances[i] = CAMERA_DISTANCE_BEHIND_PLAYER;
        animations[i] = -1; // always means no animation
    }

	// send init packet
	network->sendInitUpdate();
}

void ClientGame::handleServerActionEvent(ServerToClientPacket& updatePacket) {
    // Figure out animations
    // TODO: probably it should be the server's job, not the client's, to determine if something is moving
    for (unsigned int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
        unsigned int movementIndex = playerIndices[i];
        if (positions[movementIndex] != updatePacket.positions[movementIndex]) {
            animations[movementIndex] = WALKING;
        }
        else {
            animations[movementIndex] = NO_ANIMATION;
        }
    }
    // Handle action update (change position, camera angle, HP, etc.)
    memcpy(&positions, &updatePacket.positions, sizeof(positions));
    memcpy(&yaws, &updatePacket.yaws, sizeof(yaws));
    memcpy(&pitches, &updatePacket.pitches, sizeof(pitches));
    memcpy(&cameraDistances, &updatePacket.cameraDistances, sizeof(cameraDistances));
    // std::printf("received yaws: %f, %f, %f, %f\n", updatePacket.yaws[0], updatePacket.yaws[1], updatePacket.yaws[2], updatePacket.yaws[3]);

    // network->sendActionUpdate(); // client does not need to notify server of its action. 
}

void ClientGame::handleBulletPacket(BulletPacket& bulletPacket) {

    for (int i = 0; i < bulletPacket.count; i++) {
        // glm::vec3 gunPosition = bulletPacket.bulletTrail[i].first;
        // glm::vec3 hitPoint = bulletPacket.bulletTrail[i].second;
        // std::printf("received bullet trail gun(%f,%f,%f) -> hit(%f,%f,%f)\n", gunPosition.x, gunPosition.y, gunPosition.z, hitPoint.x, hitPoint.y, hitPoint.z);
        
        bulletQueue.push_back(bulletPacket.bulletTrail[i]);
    }
    std::printf("clientGame bullet queue size=%lu\n", bulletQueue.size());

}

void ClientGame::sendClientInputToServer()
{
    ClientToServerPacket packet;

    // Movement requests
    packet.requestForward = requestForward;
    packet.requestBackward = requestBackward;
    packet.requestLeftward = requestLeftward;
    packet.requestRightward = requestRightward;
    packet.requestJump = requestJump;
    packet.requestThrowEgg = requestThrowEgg;

    // shooting, skill
    packet.requestShoot = requestShoot;
    packet.requestAbility = requestAbility;

    // Movement angle
    packet.yaw = playerYaw;
    packet.pitch = playerPitch;
    

    // Serialize and send to server
	network->sendClientToServerPacket(packet);
}

void ClientGame::handleIssueIdentifier(IssueIdentifierUpdate issue_identifier_update) {
    client_id = issue_identifier_update.client_id;
    std::cout << "My id is " << client_id << std::endl;
}

ClientGame::~ClientGame(void) {

}
