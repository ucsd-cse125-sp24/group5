#include "bge/World.h"

namespace bge {

    void World::init() {
        // First entity will get index 0
        currMaxEntityId = 0;

        initMesh();

        positionCM = std::make_shared<ComponentManager<PositionComponent>>();
        meshCollisionCM = std::make_shared<ComponentManager<MeshCollisionComponent>>();
        velocityCM = std::make_shared<ComponentManager<VelocityComponent>>();
        jumpInfoCM = std::make_shared<ComponentManager<JumpInfoComponent>>();
        movementRequestCM = std::make_shared<ComponentManager<MovementRequestComponent>>();
        playerDataCM = std::make_shared<ComponentManager<PlayerDataComponent>>();
        speedChangeCM = std::make_shared<ComponentManager<SpeedChangeComponent>>();
        seasonAbilityStatusCM = std::make_shared<ComponentManager<SeasonAbilityStatusComponent>>();
        ballProjDataCM = std::make_shared<ComponentManager<BallProjDataComponent>>();

        healthCM = std::make_shared<ComponentManager<HealthComponent>>();

        dimensionCM = std::make_shared<ComponentManager<BoxDimensionComponent>>();

        eggHolderCM = std::make_shared<ComponentManager<EggHolderComponent>>();

        cameraCM = std::make_shared<ComponentManager<CameraComponent>>();

        std::shared_ptr<PlayerAccelerationSystem> playerAccSystem = std::make_shared<PlayerAccelerationSystem>(this, positionCM, velocityCM, movementRequestCM, jumpInfoCM, speedChangeCM);
        std::shared_ptr<MovementSystem> movementSystem = std::make_shared<MovementSystem>(this, positionCM, meshCollisionCM, velocityCM);
        std::shared_ptr<BoxCollisionSystem> boxCollisionSystem = std::make_shared<BoxCollisionSystem>(this, positionCM, eggHolderCM, dimensionCM);
        std::shared_ptr<EggMovementSystem> eggMovementSystem = std::make_shared<EggMovementSystem>(this, positionCM, eggHolderCM, movementRequestCM, playerDataCM);

        std::shared_ptr<CameraSystem> cameraSystem = std::make_shared<CameraSystem>(this, positionCM, movementRequestCM, cameraCM);
        std::shared_ptr<CollisionSystem> collisionSystem = std::make_shared<CollisionSystem>(this, positionCM, velocityCM, jumpInfoCM);
        std::shared_ptr<SeasonAbilitySystem> seasonAbilitySystem = std::make_shared<SeasonAbilitySystem>(this, movementRequestCM, playerDataCM, speedChangeCM, seasonAbilityStatusCM);

        // init players
        std::vector<glm::vec3> playerInitPositions = {  glm::vec3(11,5,17),         // hilltop
                                                        glm::vec3(15.24, 5.4, 10),  // hilltop
                                                        glm::vec3(4.5, 1.3, -5),    // house ground
                                                        glm::vec3(1.32, 7, -12.15)  // house roof
        };
        for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
            Entity newPlayer = createEntity(PLAYER);
            players[i] = newPlayer;

            // Create components
            // PositionComponent pos = PositionComponent(i*10.0f, 10.0f, -(i%2)*8.0f);
            PositionComponent pos = PositionComponent(playerInitPositions[i]);
            addComponent(newPlayer, pos);
            VelocityComponent vel = VelocityComponent(0.0f, 0.0f, 0.0f);
            addComponent(newPlayer, vel);
            std::vector<glm::vec3> collisionPoints = {glm::vec3(0, -PLAYER_Y_HEIGHT/2, 0),glm::vec3(0, PLAYER_Y_HEIGHT/2, 0),
                                                      glm::vec3(-PLAYER_X_WIDTH/2, 0, 0),glm::vec3(PLAYER_X_WIDTH/2, 0, 0),
                                                      glm::vec3(0, 0, -PLAYER_Z_WIDTH/2),glm::vec3(0, 0, PLAYER_Z_WIDTH/2)};
            std::vector<int> groundPoints = {0};
            MeshCollisionComponent meshCol = MeshCollisionComponent(collisionPoints, groundPoints);
            addComponent(newPlayer, meshCol);
            MovementRequestComponent req = MovementRequestComponent(false, false, false, false, false, false, false, 0, 0);
            addComponent(newPlayer, req);
            JumpInfoComponent jump = JumpInfoComponent(0, false);
            addComponent(newPlayer, jump);
            PlayerDataComponent playerData = PlayerDataComponent(i, AUTUMN_PLAYER, 0);
            addComponent(newPlayer, playerData);
            BoxDimensionComponent playerBoxDim = BoxDimensionComponent(PLAYER_X_WIDTH, PLAYER_Y_HEIGHT, PLAYER_Z_WIDTH);
            addComponent(newPlayer, playerBoxDim);
            CameraComponent camera = CameraComponent();
            addComponent(newPlayer, camera);
            SpeedChangeComponent speedChange = SpeedChangeComponent(0,0);
            addComponent(newPlayer, speedChange);
            SeasonAbilityStatusComponent seasonAbilityStatus = SeasonAbilityStatusComponent();
            addComponent(newPlayer, seasonAbilityStatus);

            // Add to systems
            playerAccSystem->registerEntity(newPlayer);
            movementSystem->registerEntity(newPlayer);
            boxCollisionSystem->registerEntity(newPlayer);
            cameraSystem->registerEntity(newPlayer);
            collisionSystem->registerEntity(newPlayer);
            seasonAbilitySystem->registerEntity(newPlayer);
        }

        // init egg
        egg = createEntity(EGG);

        PositionComponent pos = PositionComponent(0.73, 9, 6.36); // init Egg in front of warren bear
        addComponent(egg, pos);
        EggHolderComponent eggHolder = EggHolderComponent(INT_MIN);
        addComponent(egg, eggHolder);
        BoxDimensionComponent eggBoxDim = BoxDimensionComponent(EGG_X_WIDTH, EGG_Y_HEIGHT, EGG_Z_WIDTH);
        addComponent(egg, eggBoxDim);
        std::vector<glm::vec3> eggCollisionPoints = {glm::vec3(0, -EGG_Y_HEIGHT/2, 0),glm::vec3(0, EGG_Y_HEIGHT/2, 0),
                                                      glm::vec3(-EGG_X_WIDTH/2, 0, 0),glm::vec3(EGG_X_WIDTH/2, 0, 0),
                                                      glm::vec3(0, 0, -EGG_Z_WIDTH/2),glm::vec3(0, 0, EGG_Z_WIDTH/2)};
        MeshCollisionComponent eggMeshCol = MeshCollisionComponent(eggCollisionPoints, {0});
        addComponent(egg, eggMeshCol);
        VelocityComponent eggVel = VelocityComponent(0,-1,0);
        addComponent(egg, eggVel);

        // Add egg to systems
        eggMovementSystem->registerEntity(egg);
        boxCollisionSystem->registerEntity(egg);
        movementSystem->registerEntity(egg);   // for egg-ground collision when the egg is not carried by player

        /* 
            From positionCM's pov, players are at indices 0~3, egg is at 4 in its componentDataStorage vector.
            Client side rendering follows the same order. 
            So for the same CM, do NOT change the order of addComponent^. (which shouldn't need changing anyways)

            TODO: Maybe use ENUMS to reserve Entity IDs? 
            Fixes ordering seems easy to accidentally break. 
            ^(enums are still numbers under the hood, can't solve vector index inconsistency)
        */

        systems.push_back(playerAccSystem);
        systems.push_back(boxCollisionSystem);
        systems.push_back(movementSystem);
        systems.push_back(cameraSystem);
        systems.push_back(eggMovementSystem);
        systems.push_back(collisionSystem);
        systems.push_back(seasonAbilitySystem);
    }


    rayIntersection World::intersect(glm::vec3 p0, glm::vec3 p1, float maxT) {
        rayIntersection bestIntersection;
        bestIntersection.t=INFINITY;

        // Determine which buckets to use for checking intersection
        glm::vec3 endingPos = p0 + p1 * maxT;
        std::vector<unsigned int> bucketIndicesStart = determineBucket(p0.x, p0.z);
        std::vector<unsigned int> bucketIndicesEnd = determineBucket(endingPos.x, endingPos.z);
        unsigned int minXIndex = std::min(bucketIndicesStart[0], bucketIndicesEnd[0]);
        unsigned int maxXIndex = std::max(bucketIndicesStart[0], bucketIndicesEnd[0]);
        unsigned int minZIndex = std::min(bucketIndicesStart[1], bucketIndicesEnd[1]);
        unsigned int maxZIndex = std::max(bucketIndicesStart[1], bucketIndicesEnd[1]);

        std::unordered_set<unsigned int> mergedBucket;
        // Bullets have a long ray, so bucketStart and bucketEnd would be far apart, resulting in a large rectangle of buckets 
        // Many of them are unncessary. All we care about are buckets along ray. 
        // nice2have: Bresenham’s line generation algorithm. Fix below nxm. 
        for (unsigned int xIndex = minXIndex; xIndex <= maxXIndex; xIndex++) {
            for (unsigned int zIndex = minZIndex; zIndex <= maxZIndex; zIndex++) {
                // we store the buckets in a 1D-style, so convert this to a single index
                int bucketIndex = zIndex * MAP_BUCKET_WIDTH + xIndex;
                for (unsigned int triangleIndex : buckets[bucketIndex]) {
                    mergedBucket.insert(triangleIndex);
                }
            }
        }
        // std::cout << "ray length: " << maxT << "\t Number of buckets used: " << mergedBucket.size() << "\n";

        for (unsigned int triangleIndex : mergedBucket) {
            // get the points and the normals
            glm::vec3 A = mapVertices[mapTriangles[3 * triangleIndex + 0]];
            glm::vec3 B = mapVertices[mapTriangles[3 * triangleIndex + 1]];
            glm::vec3 C = mapVertices[mapTriangles[3 * triangleIndex + 2]];
            // get the normal - TODO precompute normals
            glm::vec3 n = glm::normalize(glm::cross((C - A), (B - A)));
            float t = (glm::dot(A, n) - glm::dot(p0, n)) / glm::dot(p1, n);
            if (t > -0.001 && t < bestIntersection.t && t < maxT + 0.001) {
                glm::vec3 iPos = p0 + t * p1;
                float area = glm::length(glm::cross(B - A, C - B)) / 2;
                float alpha = glm::length(glm::cross(B - iPos, C - iPos) / 2.0f) / area;
                float beta = glm::length(glm::cross(A - iPos, C - iPos) / 2.0f) / area;
                float gamma = glm::length(glm::cross(B - iPos, A - iPos) / 2.0f) / area;
                // currently they have slight extra give of 0.01, this is because of floating point
                // rounding. this can be adjusted
                if (alpha >= -0.01 && beta >= -0.01 && gamma >= -0.01 && alpha + beta + gamma <= 1.01) {
                    bestIntersection.t = t;
                    bestIntersection.normal = n;
                    bestIntersection.ent.type = MESH;
                }
            }
        }

        // check against player boxes here. ()

        return bestIntersection;
    }

    std::vector<unsigned int> World::determineBucket(float x, float z) {
        float bucketXDim = (maxMapXValue - minMapXValue) / MAP_BUCKET_WIDTH;
        float bucketZDim = (maxMapZValue - minMapZValue) / MAP_BUCKET_WIDTH;

        float xIndexFloat = (x - minMapXValue) / bucketXDim;
        float zIndexFloat = (z - minMapZValue) / bucketZDim;

        // if we're too close to the edge of the map we might just barely end up in a bucket that doesn't exist,
        // and we can never be completely precise with floats, so make sure we're not very far off the expected range
        // and then fix it to be in the expected range
        if (xIndexFloat >= MAP_BUCKET_WIDTH) xIndexFloat = MAP_BUCKET_WIDTH - 1;
        if (zIndexFloat >= MAP_BUCKET_WIDTH) zIndexFloat = MAP_BUCKET_WIDTH - 1;
        if (xIndexFloat < 0) xIndexFloat = 0;
        if (zIndexFloat < 0) zIndexFloat = 0;

        unsigned int xIndex = static_cast<unsigned int>(xIndexFloat);
        unsigned int zIndex = static_cast<unsigned int>(zIndexFloat);
  
        return { xIndex, zIndex };
    }

    void World::initMesh() {
        Assimp::Importer importer;
        std::string mapFilePath = (std::string)(PROJECT_PATH) + "/server/models/collision-map-fixed.obj";
        
        const aiScene* scene = importer.ReadFile(mapFilePath,
            ASSIMP_IMPORT_FLAGS);
        if (scene == nullptr) {
            std::cerr << "Unable to load 3d model from path " << mapFilePath << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Loaded environment model\n";

        minMapXValue = 0;
        maxMapXValue = 0;
        minMapZValue = 0;
        maxMapZValue = 0;

        unsigned int highestMeshIndex = scene->mNumMeshes;

        // Load meshes into ModelComposite data structures
        for (unsigned int i = 0; i < highestMeshIndex; i++) {
            aiMesh& mesh = *scene->mMeshes[i];
            // load vertices
            for (unsigned int j = 0; j < mesh.mNumVertices; j++) {
                const aiVector3D& vertex = mesh.mVertices[j];
                mapVertices.push_back(glm::vec3(vertex[0], vertex[1], vertex[2]));
                if (vertex[0] < minMapXValue) {
                    minMapXValue = vertex[0];
                } else if (vertex[0] > maxMapXValue) {
                    maxMapXValue = vertex[0];
                }
                if (vertex[2] < minMapZValue) {
                    minMapZValue = vertex[2];
                } else if (vertex[2] > maxMapZValue) {
                    maxMapZValue = vertex[2];
                }
            }
        }
        for (unsigned int i = 0; i < highestMeshIndex; i++) {
            aiMesh& mesh = *scene->mMeshes[i];
            // load triangles
            for (unsigned int j = 0; j < mesh.mNumFaces; j++) {
                const aiFace& face = mesh.mFaces[j];

                // Add vertex indices to the main triangle vector
                int triangleIndex = mapTriangles.size() / 3;
                mapTriangles.push_back(face.mIndices[0]);
                mapTriangles.push_back(face.mIndices[1]);
                mapTriangles.push_back(face.mIndices[2]);

                // A, B, and C are the vertices of this triangle
                glm::vec3 A = mapVertices[face.mIndices[0]];
                glm::vec3 B = mapVertices[face.mIndices[1]];
                glm::vec3 C = mapVertices[face.mIndices[2]];

                std::vector<unsigned int> bucketIndicesA = determineBucket(A.x, A.z);
                std::vector<unsigned int> bucketIndicesB = determineBucket(B.x, B.z);
                std::vector<unsigned int> bucketIndicesC = determineBucket(C.x, C.z);

                // We want to put the triangles into every bucket they cover (usually this should just be one bucket)
                // we do this by putting the triangle into all buckets in the rectangle from the minimum x index to the maximum x index
                // and from the minimum z index to the maximum z index
                // this may occasionally lead to putting a triangle into a bucket that it doesn't actually cover,
                // but we don't care very much (small performance loss),
                // and it should never lead to a triangle not being in a bucket it should be in
                unsigned int minXIndex = std::min({bucketIndicesA[0], bucketIndicesB[0], bucketIndicesC[0]});
                unsigned int maxXIndex = std::max({bucketIndicesA[0], bucketIndicesB[0], bucketIndicesC[0]});
                unsigned int minZIndex = std::min({bucketIndicesA[1], bucketIndicesB[1], bucketIndicesC[1]});
                unsigned int maxZIndex = std::max({bucketIndicesA[1], bucketIndicesB[1], bucketIndicesC[1]});
                for (unsigned int xIndex = minXIndex; xIndex <= maxXIndex; xIndex++) {
                    for (unsigned int zIndex = minZIndex; zIndex <= maxZIndex; zIndex++) {
                        // we store the buckets in a 1D-style, so convert this to a single index
                        int bucketIndex = zIndex * MAP_BUCKET_WIDTH + xIndex;
                        // add the triangle to the bucket
                        /*buckets[bucketIndex].push_back(face.mIndices[0]);
                        buckets[bucketIndex].push_back(face.mIndices[1]);
                        buckets[bucketIndex].push_back(face.mIndices[2]);*/
                        buckets[bucketIndex].insert(triangleIndex);
                    }
                }
            }
        }

        std::cout << "loaded vertices and triangles\n";
    }

    Entity World::createEntity(EntityType type) {
        Entity newEntity = Entity();
        newEntity.id = currMaxEntityId;
        newEntity.type = type;
        currMaxEntityId++;
        entities.insert(newEntity);
        return newEntity;
    }

    void World::deleteEntity(Entity entity) {
        // TODO: remove an entity from all system, component manager, etc.
    }

    void World::addComponent(Entity e, PositionComponent c) {
        positionCM->add(e, c);
    }
    void World::addComponent(Entity e, MeshCollisionComponent c) {
        meshCollisionCM->add(e, c);
    }
    void World::addComponent(Entity e, VelocityComponent c) {
        velocityCM->add(e, c);
    }
    void World::addComponent(Entity e, JumpInfoComponent c) {
        jumpInfoCM->add(e, c);
    }
    void World::addComponent(Entity e, MovementRequestComponent c) {
        movementRequestCM->add(e, c);
    }
    void World::addComponent(Entity e, HealthComponent c) {
        healthCM->add(e, c);
    }
    void World::addComponent(Entity e, BoxDimensionComponent c) {
        dimensionCM->add(e, c);   
    }
    void World::addComponent(Entity e, EggHolderComponent c) {
        eggHolderCM->add(e, c);
    }
    void World::addComponent(Entity e, PlayerDataComponent c){
        playerDataCM->add(e, c);
    }
    void World::addComponent(Entity e, CameraComponent c) {
        cameraCM->add(e, c);
    }
    void World::addComponent(Entity e, SpeedChangeComponent c) {
        speedChangeCM->add(e, c);
    }
    void World::addComponent(Entity e, SeasonAbilityStatusComponent c) {
        seasonAbilityStatusCM->add(e, c);
    }

    template<typename ComponentType>
    void World::deleteComponent(Entity e, ComponentType c) {
        // Hopefully this actually prints out useful information, apparently the behavior of name() depends on the implementation
        // and may not be human readable
        std::cout << "Error: no remove component function for this component type: " << typeid(ComponentType).name() << std::endl;
        assert(false);
    }
    template<>
    void World::deleteComponent(Entity e, PositionComponent c) {
        positionCM->remove(e);
    }
    template<>
    void World::deleteComponent(Entity e, VelocityComponent c) {
        velocityCM->remove(e);
    }
    template<>
    void World::deleteComponent(Entity e, MovementRequestComponent c) {
        movementRequestCM->remove(e);
    }
    template<>
    void World::deleteComponent(Entity e, JumpInfoComponent c) {
        jumpInfoCM->remove(e);
    }

    void World::updateAllSystems() {
        // this needs to be a reference because the elements in systems are unique_ptrs
        for (auto& s : systems) {
            s->update();
        }
    }

    void World::printDebug() {
    }

    void World::updatePlayerInput(unsigned int player, float pitch, float yaw, bool forwardRequested, bool backwardRequested, bool leftRequested, bool rightRequested, bool jumpRequested, bool throwEggRequested, bool seasonAbilityRequested) {
        MovementRequestComponent& req = movementRequestCM->lookup(players[player]);

        req.pitch = pitch;
        req.yaw = yaw;
        req.forwardRequested = forwardRequested;
        req.backwardRequested = backwardRequested;
        req.leftRequested = leftRequested;
        req.rightRequested = rightRequested;
        req.jumpRequested = jumpRequested;
        req.throwEggRequested = throwEggRequested;
        req.seasonAbilityRequested = seasonAbilityRequested;
    }

    void World::createProjectile() {

        Entity newProjectile = createEntity(PROJECTILE);

        PositionComponent pos = PositionComponent(0.0f, 0.0f, 0.0f);
        addComponent(newProjectile, pos);
        VelocityComponent vel = VelocityComponent(0.0f, 0.0f, 0.0f);
        addComponent(newProjectile, vel);

        // projectileVsPlayerHandler->registerEntity(newProjectile); // todo: register it in Movement system. 
    }

    void World::fillInGameData(ServerToClientPacket& packet) {
        std::vector<PositionComponent> positions = positionCM->getAllComponents();
        for (int i = 0; i < NUM_MOVEMENT_ENTITIES; i++) {
            packet.positions[i] = positions[i].position;
        }
        std::vector<MovementRequestComponent> requests = movementRequestCM->getAllComponents();
        for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
            packet.pitches[i] = requests[i].pitch;
            packet.yaws[i] = requests[i].yaw;
        }
        std::vector<CameraComponent> cameras = cameraCM->getAllComponents();
        for (int i = 0; i < cameras.size(); i++) {
            packet.cameraDistances[i] = cameras[i].distanceBehindPlayer;
        }
        std::vector<VelocityComponent> velocities = velocityCM->getAllComponents();
        for (int i = 0; i < velocities.size(); i++) {
            packet.movementEntityStates[i][ON_GROUND] = velocities[i].onGround;
            packet.movementEntityStates[i][MOVING_HORIZONTALLY] = velocities[i].velocity.x != 0 || velocities[i].velocity.z != 0;
        }
    }
} 