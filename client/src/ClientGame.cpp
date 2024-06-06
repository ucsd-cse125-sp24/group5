#include "ClientGame.h"

ClientGame::ClientGame()
{
    network = std::make_unique<ClientNetwork>(this);
    client_id = 0;
    std::cout << "Initializing client game world...\n";
    for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
        positions[i] = glm::vec3(i*10.0f, 0.0f, -(i%2)*8.0f);
        yaws[i] = -90.0f;
        animations[i] = -1; // always means no animation
    }
    for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
        cameraDistances[i] = CAMERA_DISTANCE_BEHIND_PLAYER;
        healths[i] = 100;
        scores[i] = 0;

    }

	// send init packet
	network->sendInitUpdate();
}

// This isn't part of the constructor since we need the projIndices to be set already, which happens in Client.cpp
void ClientGame::initializeParticleEmitters() {
    // Spring leaf particles
    ambientParticleEmitters[0]=std::make_unique<sge::DiskParticleEmitterEntity>
    (2.0f, 0.3f, 0.0f, 4000, 4000, ambientColorProbs[0], ambientStartingColors[0], ambientEndingColors[0],
    glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.0f, glm::vec3(0.0f, 0.002f, 0.0f), 
    glm::vec3(0.0f, 0.0f, 0.0f), 50.0f);
    ambientParticleEmitters[0]->setActive(true);

    // Summer leaf particles
    ambientParticleEmitters[1] = std::make_unique<sge::DiskParticleEmitterEntity>
        (1.0f, 0.3f, 0.0f, 4000, 4000, ambientColorProbs[1], ambientStartingColors[1], ambientEndingColors[1],
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.0f, glm::vec3(0.0f, -0.002f, 0.0f),
            glm::vec3(0.0f, 8.0f, 0.0f), 50.0f);
    ambientParticleEmitters[1]->setActive(true);

    // Autum leaf particles
    ambientParticleEmitters[2] = std::make_unique<sge::DiskParticleEmitterEntity>
        (7.0f, 0.3f, 0.0f, 4000, 4000, ambientColorProbs[2], ambientStartingColors[2], ambientEndingColors[2],
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.0f, glm::vec3(0.0f, -0.002f, 0.0f),
            glm::vec3(0.0f, 8.0f, 0.0f), 50.0f);
    ambientParticleEmitters[2]->setActive(true);

    // Winter snow particles
    ambientParticleEmitters[3] = std::make_unique<sge::DiskParticleEmitterEntity>
        (25.0f, 0.25f, 0.0f, 7000, 7000, ambientColorProbs[3], ambientStartingColors[3], ambientEndingColors[3],
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.0f, glm::vec3(0.0f, -0.002f, 0.0f),
            glm::vec3(0.0f, 15.0f, 0.0f), 50.0f);
    ambientParticleEmitters[3]->setActive(true);

    for(unsigned int i = 0; i < NUM_PROJ_TYPES; i++) {
        for (unsigned int j = 0; j < NUM_EACH_PROJECTILE; j++) {
            unsigned int movementIndex = projIndices[i * NUM_EACH_PROJECTILE + j];
            projParticleEmitters[i * NUM_EACH_PROJECTILE + j] = std::make_unique<sge::ParticleEmitterEntity>(4.0f,
                                                                0.5f,
                                                                0.0f,
                                                                1000,
                                                                1000,
                                                                projColorProbs[i],
                                                                projStartingColors[i],
                                                                projEndingColors[i],
                                                                glm::vec3(0.1f, 0.1f, 0.1f),
                                                                glm::vec3(-0.5f, -0.5f, -0.5f),
                                                                1.0f,
                                                                0.0f,
                                                                glm::vec3(0.0f, 0.005f, 0.0f),
                                                                movementIndex,
                                                                glm::vec3(0.0f, 0.0f, 0.0f));
            projParticleEmitters[i * NUM_EACH_PROJECTILE + j]->setActive(false);
            projExplosionEmitters[i * NUM_EACH_PROJECTILE + j] = std::make_unique<sge::ParticleEmitterEntity>(0.0f,
                                                                 0.5f,
                                                                 0.0f,
                                                                 1500,
                                                                 250,
                                                                 projColorProbs[i],
                                                                 projStartingColors[i],
                                                                 projEndingColors[i],
                                                                 glm::vec3(0.1f, 0.1f, 0.1f),
                                                                 glm::vec3(-0.5f, -0.5f, -0.5f),
                                                                 1.0f,
                                                                 0.0f,
                                                                 glm::vec3(0.0f, 0.0f, 0.0f),
                                                                 movementIndex,
                                                                 glm::vec3(0.0f, 0.0f, 0.0f));
            projExplosionEmitters[i * NUM_EACH_PROJECTILE + j]->setActive(false);
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
        if (movementEntityStates[movementIndex][IS_DANCING]) {
            // animations[movementIndex] = DANCING; // todo: use this after all 4 dance animations are ready @Joanne
            animations[movementIndex] = WALKING;   // moonwalk, for testing only
        }
    }
    for (unsigned int i = 0; i < NUM_TOTAL_PROJECTILES; i++) {
        unsigned int movementIndex = projIndices[i];
        if (movementEntityStates[movementIndex][EXPLODING]) {
            projExplosionEmitters[i]->explode();
        }
    }
}

bool ClientGame::shouldRenderBombTicks() {
    return (client_id == eggHolderId || bombIsThrown) && eggIsDanceBomb && !danceInAction;
}

bool ClientGame::shouldPlayBombTicking() {
    if (bombTickingPlaying) {
        return false;
    }

    if (eggIsDanceBomb && detonationMiliSecs < 2000) {
        bombTickingPlaying = true;
        return true;
    }

    return false;
}

bool ClientGame::shouldPlayDanceSong() {
    if (danceSongPlaying) {
        return false;
    }

    if (danceInAction) {
        danceSongPlaying = true;
        return true;
    }

    return false;
}

bool ClientGame::shouldStopDanceSong() {
    if (danceSongStopped) {
        return false;
    }
    if (danceSongPlaying && !danceInAction && !eggIsDanceBomb) {
        danceSongStopped = true;

        bombTickingPlaying = danceSongPlaying = danceSongStopped = false; // reset guards
        return true;
    }
    return false;
}


void ClientGame::handleServerActionEvent(ServerToClientPacket& updatePacket) {
    // Handle action update (change position, camera angle, HP, etc.)
    memcpy(&positions, &updatePacket.positions, sizeof(positions));
    memcpy(&yaws, &updatePacket.yaws, sizeof(yaws));
    memcpy(&pitches, &updatePacket.pitches, sizeof(pitches));
    memcpy(&cameraDistances, &updatePacket.cameraDistances, sizeof(cameraDistances));
    memcpy(&projActive, &updatePacket.active, sizeof(projActive));
    // std::printf("received yaws: %f, %f, %f, %f\n", updatePacket.yaws[0], updatePacket.yaws[1], updatePacket.yaws[2], updatePacket.yaws[3]);
    memcpy(&healths, &updatePacket.healths, sizeof(healths));
    memcpy(&scores, &updatePacket.scores, sizeof(scores));
    memcpy(&currentSeason, &updatePacket.currentSeason, sizeof(currentSeason));   
    memcpy(&seasonBlend, &updatePacket.seasonBlend, sizeof(seasonBlend));
    memcpy(&eggIsDanceBomb, &updatePacket.eggIsDanceBomb, sizeof(eggIsDanceBomb));
    memcpy(&danceInAction, &updatePacket.danceInAction, sizeof(danceInAction));
    memcpy(&eggHolderId, &updatePacket.eggHolderId, sizeof(eggHolderId));
    memcpy(&detonationMiliSecs, &updatePacket.detonationMiliSecs, sizeof(detonationMiliSecs));
    memcpy(&gameDurationInSeconds, &updatePacket.gameDurationInSeconds, sizeof(gameDurationInSeconds));
    bombIsThrown = updatePacket.bombIsThrown;

    updateAnimations(updatePacket.movementEntityStates);

    // network->sendActionUpdate(); // client does not need to notify server of its action. 
}


void ClientGame::handleLobbySelectionPacket(LobbyServerToClientPacket& lobbyPacket) {
    for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
        characterUID[i] = lobbyPacket.playersCharacter[i];
        browsingCharacterUID[i] = lobbyPacket.playersBrowsingCharacter[i];
        teams[i] = lobbyPacket.teams[i];
        //std::cout << "Player " << i << " browsing " << browsingCharacterUID[i] << ", select " << characterUID[i] << std::endl;
    }

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

void ClientGame::handleGameEndPacket(GameEndPacket& gameEndPacket) {
    gameOver = gameEndPacket.gameOver;
    winner = gameEndPacket.winner;
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
    packet.requestBomb = requestBomb;

    // shooting, skill
    packet.requestShoot = requestShoot;
    packet.requestAbility = requestAbility;

    // completely reset player and egg position
    packet.requestReset = requestReset;

    // Movement angle
    packet.yaw = playerYaw;
    packet.pitch = playerPitch;
    

    // Serialize and send to server
	network->sendClientToServerPacket(packet);
}

void ClientGame::sendLobbySelectionToServer(int browsingCharacterUID, int selectedCharacterUID) {
    LobbyClientToServerPacket packet;

    packet.characterUID = selectedCharacterUID;
    packet.browsingCharacterUID = browsingCharacterUID;


    // Serialize and send to server
    network->sendLobbyClientToServer(packet);
}

void ClientGame::handleIssueIdentifier(IssueIdentifierUpdate issue_identifier_update) {
    client_id = issue_identifier_update.client_id;
    std::cout << "My id is " << client_id << std::endl;
}

ClientGame::~ClientGame(void) {

}
