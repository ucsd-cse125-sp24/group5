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

// This isn't part of the constructor since we need the projIndices to be set already, which happens in Client.cpp
void ClientGame::initializeParticleEmitters() {
    for(unsigned int i = 0; i < 4; i++) {
        // TODO: fix this, disk particle emitter not working
        ambientParticleEmitters[i]=std::make_unique<sge::DiskParticleEmitterEntity>
        (2.0f, 10.0f, 0.0f, 100, ambientColorProbs[i], ambientStartingColors[i], ambientEndingColors[i],
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.0f, glm::vec3(0.0f, -0.0000f, 0.0f), 
        glm::vec3(0.0f, 3.0f, 0.0f), 50.0f);
        // ambientParticleEmitters[i]=std::make_unique<sge::DiskParticleEmitterEntity>
        // (2.0f,
        //                       0.5f,
        //                       0.0f,
        //                       10000,
        //                       ambientColorProbs[i],
        //                       ambientStartingColors[i],
        //                       ambientEndingColors[i],
        //                       glm::vec3(0.0f, 0.0f, 0.0f),
        //                       glm::vec3(0.0f, 0.0f, 0.0f),
        //                       1,
        //                       0,
        //                       glm::vec3(0.0f, 0.005f, 0.0f),
        //                       glm::vec3(0.0f, 0.0f, 0.0f),
        //                       10);
         ambientParticleEmitters[i]->setActive(true);
    }

    for(unsigned int i = 0; i < NUM_PROJ_TYPES; i++) {
        for (unsigned int j = 0; j < NUM_EACH_PROJECTILE; j++) {
            unsigned int movementIndex = projIndices[i * NUM_EACH_PROJECTILE + j];
            projParticleEmitters[i * NUM_EACH_PROJECTILE + j] = makeProjParticleEmitterEntity(projColorProbs[i],
                projStartingColors[i],
                projEndingColors[i],
                movementIndex
            );
            projParticleEmitters[i * NUM_EACH_PROJECTILE + j]->setActive(false);
        }
    }
}

void ClientGame::updateAnimations(std::bitset<NUM_STATES> movementEntityStates[]) {
    for (unsigned int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
        unsigned int movementIndex = playerIndices[i];
        if (movementEntityStates[movementIndex][MOVING_HORIZONTALLY]) {
            animations[movementIndex] = WALKING;
        }
        else if (!movementEntityStates[movementIndex][ON_GROUND]) {
            animations[movementIndex] = JUMPING;
        } else {
            animations[movementIndex] = STILL;
        }
        // shooting and abilities animation
        if (movementEntityStates[movementIndex][IS_SHOOTING]) {
            animations[movementIndex] = SHOOTING;
        }
    }
    for (unsigned int i = 0; i < NUM_TOTAL_PROJECTILES; i++) {
        unsigned int movementIndex = projIndices[i];
        if (movementEntityStates[movementIndex][EXPLODING]) {
            projParticleEmitters[i]->explode();
        }
    }
}

void ClientGame::handleServerActionEvent(ServerToClientPacket& updatePacket) {
    // Handle action update (change position, camera angle, HP, etc.)
    memcpy(&positions, &updatePacket.positions, sizeof(positions));
    memcpy(&yaws, &updatePacket.yaws, sizeof(yaws));
    memcpy(&pitches, &updatePacket.pitches, sizeof(pitches));
    memcpy(&cameraDistances, &updatePacket.cameraDistances, sizeof(cameraDistances));
    memcpy(&projActive, &updatePacket.active, sizeof(projActive));
    // std::printf("received yaws: %f, %f, %f, %f\n", updatePacket.yaws[0], updatePacket.yaws[1], updatePacket.yaws[2], updatePacket.yaws[3]);

    updateAnimations(updatePacket.movementEntityStates);

    // network->sendActionUpdate(); // client does not need to notify server of its action. 
}

void ClientGame::handleBulletPacket(BulletPacket& bulletPacket) {

    for (int i = 0; i < bulletPacket.count; i++) {
        // glm::vec3 gunPosition = bulletPacket.bulletTrail[i].first;
        // glm::vec3 hitPoint = bulletPacket.bulletTrail[i].second;
        // std::printf("received bullet trail gun(%f,%f,%f) -> hit(%f,%f,%f)\n", gunPosition.x, gunPosition.y, gunPosition.z, hitPoint.x, hitPoint.y, hitPoint.z);
        
        bulletQueue.push_back(BulletToRender(bulletPacket.bulletTrail[i].start, bulletPacket.bulletTrail[i].end, BULLET_FRAMES));

        // if my client is one of the shooters, play shooting sound
        if (client_id == bulletPacket.bulletTrail->shooterId) {
            sound::soundManager->shootingSound();
        
            // if i hit another player
            if (bulletPacket.bulletTrail->playerHit != -1) {
                shootingEmo = 1;
            }
        }
    }
    // std::printf("clientGame bullet queue size=%lu\n", bulletQueue.size());
}

void ClientGame::updateShootingEmo() {
    // no players hit, crosshair remain normal
    if (shootingEmo == 0) {
        return;
    }
    // if hit another player, crosshair grows the next ? frames (then reset to normal)
    shootingEmo = (shootingEmo + 1) % BULLET_FRAMES;

    // std::printf("shooting emo= %d\n", shootingEmo);

}

void ClientGame::updateBulletQueue() {
    // remove bullets that were rendered for >BULLET_FRAMES frames
    while (!bulletQueue.empty()) {
        BulletToRender& front = bulletQueue.front();
        if (front.framesToRender-- <= 0) {
            bulletQueue.pop_front();
        }
        else {
            break;
        }
    }
    // printf("bulletQueue size after cleanup= %lu\n", bulletQueue.size());

    // move bullet segment to its next position
    for (BulletToRender& b : bulletQueue) {
        b.start += b.delta;
        b.currEnd += b.delta;
    }
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

std::unique_ptr<sge::ParticleEmitterEntity> makeProjParticleEmitterEntity(std::vector<float> colorProbs,
    std::vector<glm::vec4> initColors,
    std::vector<glm::vec4> endColors,
    size_t positionIndex) {
    return std::make_unique<sge::ParticleEmitterEntity>(4.0f,
        0.5f,
        0.0f,
        1000,
        colorProbs,
        initColors,
        endColors,
        glm::vec3(0.1f, 0.1f, 0.1f),
        glm::vec3(-0.5f, -0.5f, -0.5f),
        1.0f,
        0.0f,
        glm::vec3(0.0f, 0.005f, 0.0f),
        positionIndex,
        glm::vec3(0.0f, 0.0f, 0.0f));

}