#include "bge/World.h"

namespace bge {

    void World::init() {
        // First entity will get index 0
        currMaxEntityId = 0;

        initMesh();

        // Put objects here to make them disappear
        voidLocation = glm::vec3(0.0f, -100.0f, 0.0f);

        positionCM = std::make_shared<ComponentManager<PositionComponent>>();
        meshCollisionCM = std::make_shared<ComponentManager<MeshCollisionComponent>>();
        velocityCM = std::make_shared<ComponentManager<VelocityComponent>>();
        jumpInfoCM = std::make_shared<ComponentManager<JumpInfoComponent>>();
        movementRequestCM = std::make_shared<ComponentManager<MovementRequestComponent>>();
        playerDataCM = std::make_shared<ComponentManager<PlayerDataComponent>>();
        statusEffectsCM = std::make_shared<ComponentManager<StatusEffectsComponent>>();
        seasonAbilityStatusCM = std::make_shared<ComponentManager<SeasonAbilityStatusComponent>>();
        ballProjDataCM = std::make_shared<ComponentManager<BallProjDataComponent>>();

        healthCM = std::make_shared<ComponentManager<HealthComponent>>();

        boxDimensionCM = std::make_shared<ComponentManager<BoxDimensionComponent>>();

        eggHolderCM = std::make_shared<ComponentManager<EggHolderComponent>>();

        cameraCM = std::make_shared<ComponentManager<CameraComponent>>();
        lerpingCM = std::make_shared<ComponentManager<LerpingComponent>>();

        std::shared_ptr<PlayerAccelerationSystem> playerAccSystem = std::make_shared<PlayerAccelerationSystem>(this, positionCM, velocityCM, movementRequestCM, jumpInfoCM, statusEffectsCM);
        std::shared_ptr<MovementSystem> movementSystem = std::make_shared<MovementSystem>(this, positionCM, meshCollisionCM, velocityCM);
        std::shared_ptr<BoxCollisionSystem> boxCollisionSystem = std::make_shared<BoxCollisionSystem>(this, positionCM, eggHolderCM, boxDimensionCM);
        std::shared_ptr<EggMovementSystem> eggMovementSystem = std::make_shared<EggMovementSystem>(this, positionCM, eggHolderCM, movementRequestCM, playerDataCM);

        std::shared_ptr<CameraSystem> cameraSystem = std::make_shared<CameraSystem>(this, positionCM, movementRequestCM, cameraCM);
        std::shared_ptr<BulletSystem> bulletSystem = std::make_shared<BulletSystem>(this, positionCM, movementRequestCM, cameraCM, playerDataCM, healthCM);
        std::shared_ptr<SeasonAbilitySystem> seasonAbilitySystem = std::make_shared<SeasonAbilitySystem>(this, movementRequestCM, playerDataCM, seasonAbilityStatusCM, ballProjDataCM, positionCM, velocityCM, cameraCM);
        std::shared_ptr<ProjectileStateSystem> projectileStateSystem = std::make_shared<ProjectileStateSystem>(this, playerDataCM, statusEffectsCM, ballProjDataCM, positionCM, velocityCM, meshCollisionCM, healthCM);
        std::shared_ptr<SeasonEffectSystem> seasonEffectSystem = std::make_shared<SeasonEffectSystem>(this, healthCM, velocityCM, movementRequestCM, jumpInfoCM, seasonAbilityStatusCM);
        std::shared_ptr<LerpingSystem> lerpingSystem = std::make_shared<LerpingSystem>(this);

        // init players
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
            MeshCollisionComponent meshCol = MeshCollisionComponent(collisionPoints, groundPoints, true);
            addComponent(newPlayer, meshCol);
            MovementRequestComponent req = MovementRequestComponent(false, false, false, false, false, false, false, false, false, 0, -90);
            addComponent(newPlayer, req);
            JumpInfoComponent jump = JumpInfoComponent(0, false);
            addComponent(newPlayer, jump);
            time_t timer;
            time(&timer);
            PlayerDataComponent playerData = PlayerDataComponent(i, AUTUMN_PLAYER, 0, 0);
            addComponent(newPlayer, playerData);
            BoxDimensionComponent playerBoxDim = BoxDimensionComponent(PLAYER_X_WIDTH, PLAYER_Y_HEIGHT, PLAYER_Z_WIDTH);
            addComponent(newPlayer, playerBoxDim);
            CameraComponent camera = CameraComponent();
            addComponent(newPlayer, camera);
            HealthComponent health = HealthComponent(PLAYER_MAX_HEALTH);
            addComponent(newPlayer, health);
            StatusEffectsComponent statusEffects = StatusEffectsComponent(0,0,0);
            addComponent(newPlayer, statusEffects);
            SeasonAbilityStatusComponent seasonAbilityStatus = SeasonAbilityStatusComponent();
            addComponent(newPlayer, seasonAbilityStatus);

            // Add to systems
            playerAccSystem->registerEntity(newPlayer);
            movementSystem->registerEntity(newPlayer);
            boxCollisionSystem->registerEntity(newPlayer);
            cameraSystem->registerEntity(newPlayer);
            bulletSystem->registerEntity(newPlayer);
            seasonAbilitySystem->registerEntity(newPlayer);
            seasonEffectSystem->registerEntity(newPlayer);
            lerpingSystem->registerEntity(newPlayer);
        }

        // init egg
        egg = createEntity(EGG);

        PositionComponent pos = PositionComponent(eggInitPosition); // init Egg in front of warren bear
        addComponent(egg, pos);
        EggHolderComponent eggHolder = EggHolderComponent(INT_MIN);
        addComponent(egg, eggHolder);
        BoxDimensionComponent eggBoxDim = BoxDimensionComponent(EGG_X_WIDTH, EGG_Y_HEIGHT, EGG_Z_WIDTH);
        addComponent(egg, eggBoxDim);
        std::vector<glm::vec3> eggCollisionPoints = {glm::vec3(0, -EGG_Y_HEIGHT/2, 0),glm::vec3(0, EGG_Y_HEIGHT/2, 0),
                                                      glm::vec3(-EGG_X_WIDTH/2, 0, 0),glm::vec3(EGG_X_WIDTH/2, 0, 0),
                                                      glm::vec3(0, 0, -EGG_Z_WIDTH/2),glm::vec3(0, 0, EGG_Z_WIDTH/2)};
        MeshCollisionComponent eggMeshCol = MeshCollisionComponent(eggCollisionPoints, {0}, true);
        addComponent(egg, eggMeshCol);
        VelocityComponent eggVel = VelocityComponent(0,-1,0);
        addComponent(egg, eggVel);

        // Add egg to systems
        eggMovementSystem->registerEntity(egg);
        boxCollisionSystem->registerEntity(egg);
        movementSystem->registerEntity(egg);   // for egg-ground collision when the egg is not carried by player
        lerpingSystem->registerEntity(egg);

        /* 
            From positionCM's pov, players are at indices 0~3, egg is at 4 in its componentDataStorage vector.
            Client side rendering follows the same order. 
            So for the same CM, do NOT change the order of addComponent^. (which shouldn't need changing anyways)

            TODO: Maybe use ENUMS to reserve Entity IDs? 
            Fixes ordering seems easy to accidentally break. 
            ^(enums are still numbers under the hood, can't solve vector index inconsistency)
        */

        // Init all ball projectiles (they start out inactive then we'll make them active as we need them)
        for (unsigned int i = 0; i < NUM_PROJ_TYPES; i++) {
            for (unsigned int j = 0; j < NUM_EACH_PROJECTILE; j++) {
                Entity newProj = createEntity(PROJECTILE);
                ballProjectiles[i][j] = newProj;

                // Position starts below the map where they can't be seen
                PositionComponent pos = PositionComponent(voidLocation);
                addComponent(newProj, pos);
                VelocityComponent vel = VelocityComponent(0.0f, 0.0f, 0.0f);
                addComponent(newProj, vel);
                std::vector<glm::vec3> collisionPoints = { glm::vec3(0, -PROJ_Y_HEIGHT / 2, 0),glm::vec3(0, PROJ_Y_HEIGHT / 2, 0),
                                                          glm::vec3(-PROJ_X_WIDTH / 2, 0, 0),glm::vec3(PROJ_X_WIDTH / 2, 0, 0),
                                                          glm::vec3(0, 0, -PROJ_Z_WIDTH / 2),glm::vec3(0, 0, PROJ_Z_WIDTH / 2) };
                // All points are "ground points" since we're using this to determine if we've collided at all
                std::vector<int> groundPoints = { 0, 1, 2, 3, 4, 5 };
                MeshCollisionComponent meshCol = MeshCollisionComponent(collisionPoints, groundPoints, false);
                addComponent(newProj, meshCol);
                BoxDimensionComponent boxDim = BoxDimensionComponent(PROJ_X_WIDTH, PROJ_Y_HEIGHT, PROJ_Z_WIDTH);
                addComponent(newProj, boxDim);
                BallProjDataComponent data = BallProjDataComponent((BallProjType)i);
                addComponent(newProj, data);

                projIndices.push_back(movementSystem->size());
                movementSystem->registerEntity(newProj);
                projectileStateSystem->registerEntity(newProj);
                boxCollisionSystem->registerEntity(newProj);
            }
        }

        currentSeason = SPRING_SEASON;
        seasonCounter = 0;

        // Process player input
        systems.push_back(playerAccSystem);
        // Process position of the player camera
        systems.push_back(cameraSystem);
        // Process seasonal effects
        systems.push_back(seasonEffectSystem);
        // Process bullet shooting
        systems.push_back(bulletSystem);
        // Process seasonal skill shooting
        systems.push_back(seasonAbilitySystem);
        // Process seasonal skill hits
        systems.push_back(projectileStateSystem);
        // Process collision with boxes
        systems.push_back(boxCollisionSystem);
        // Process collision with world mesh
        systems.push_back(movementSystem);
        // Process movement of the egg
        systems.push_back(eggMovementSystem);

        // Process lerping entities' position update
        systems.push_back(lerpingSystem);

        gameOver = false;

        // initialize all players' character selection
        for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
            charactersUID[i] = NO_CHARACTER;
        }

        // initialize all players' initial browsing character selection
        for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
            browsingCharactersUID[i] = SPRING_CHARACTER;
        }

        // initialize all team setup
        for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
            if (i % 2 == 0) {
                teammates[i] = i + 1;
            }
            else {
                teammates[i] = i - 1;
            }
        }
        
    }

    void World::resetPlayer(unsigned int playerId) {
        PositionComponent& pos = positionCM->lookup(players[playerId]);
        pos.position = playerInitPositions[playerId];
        pos.isLerping = false;

        VelocityComponent& vel = velocityCM->lookup(players[playerId]);
        vel.velocity = glm::vec3(0, 0, 0);
        vel.timeOnGround = 0;
        vel.onGround = false;

        JumpInfoComponent& jump = jumpInfoCM->lookup(players[playerId]);
        jump.doubleJumpUsed = 0;
    }

    // Reset the egg's state, including returning it to its inital location
    void World::resetEgg() {
        // reset egg
        PositionComponent& pos = positionCM->lookup(egg);
        pos.position = eggInitPosition;

        VelocityComponent& vel = velocityCM->lookup(egg);
        vel.velocity = glm::vec3(0, 0, 0);
        vel.timeOnGround = 0;
        vel.onGround = false;

        EggHolderComponent& eggHolder = eggHolderCM->lookup(egg);
        eggHolder.holderId = INT_MIN;
        eggHolder.throwerId = INT_MIN;
        eggHolder.isThrown = false;
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

    rayIntersection World::intersectRayBox(glm::vec3 origin, glm::vec3 direction, float maxT) {

        rayIntersection bestIntersection;
        bestIntersection.t = INFINITY;
        bestIntersection.ent.id = -1; // no player hit 

        Entity targets[] = {players[0], players[1], players[2], players[3], egg};

        for (Entity target: targets) {

            // target box
            PositionComponent& pos = positionCM->lookup(target);
            BoxDimensionComponent& dim = boxDimensionCM->lookup(target);
            glm::vec3 min = pos.position - dim.halfDimension;
            glm::vec3 max = pos.position + dim.halfDimension;
       
            // Ray
            float tFar = INFINITY;
            float tNear = -tFar;

            // check ray's x direction against the min and max yz-plane, etc. 
            for (int i = 0; i < 3; i++) {
                if (std::abs(direction[i]) < 0.0001f) {
                    if (origin[i] < min[i] || origin[i] > max[i]) {
                        tNear = INFINITY; // no hit, cuz ur out of the plane
                        break;
                    }
                }
                else {
                    // https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html
                    float t1 = (min[i] - origin[i]) / direction[i];
                    float t2 = (max[i] - origin[i]) / direction[i];
                    if (t1 > t2) {
                        std::swap(t1, t2);
                    }
                    tNear = std::max(tNear, t1);
                    tFar = std::min(tFar, t2);

                    if (tNear > tFar || tFar < 0.0f) {
                        tNear = INFINITY;
                        break;
                    }
                }
            }

            // hits, is it the closest target?
            if (tNear < maxT && tNear < bestIntersection.t) {
                bestIntersection.t = tNear;
                bestIntersection.ent = target;
            }
            
        }

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
        std::string mapFilePath = (std::string)(PROJECT_PATH) + SetupParser::getValue("collision-map");
        
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
        minMapYValue = 0;
        maxMapYValue = 0;

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
                if (vertex[1] < minMapYValue) {
                    minMapYValue = vertex[1];
                } else if (vertex[1] > maxMapYValue) {
                    maxMapYValue = vertex[1];
                }
                if (vertex[2] < minMapZValue) {
                    minMapZValue = vertex[2];
                }
                else if (vertex[2] > maxMapZValue) {
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
        boxDimensionCM->add(e, c);   
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
    void World::addComponent(Entity e, StatusEffectsComponent c) {
        statusEffectsCM->add(e, c);
    }
    void World::addComponent(Entity e, SeasonAbilityStatusComponent c) {
        seasonAbilityStatusCM->add(e, c);
    }
    void World::addComponent(Entity e, BallProjDataComponent c) {
        ballProjDataCM->add(e, c);
    }
    void World::addComponent(Entity e, LerpingComponent c) {
        lerpingCM->add(e, c);
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
    template<>
    void World::deleteComponent(Entity e, BallProjDataComponent c) {
        ballProjDataCM->remove(e);
    }
    template<>
    void World::deleteComponent(Entity e, LerpingComponent c) {
        lerpingCM->remove(e);
    }

    void World::startWorldTimer() {
        time(&worldTimer);
    }

    void World::updateAllSystems() {

        double gameDurationInSeconds;

        if (!gameOver) {
            // this needs to be a reference because the elements in systems are unique_ptrs
            for (auto& s : systems) {
                s->update();
            }
            gameDurationInSeconds = difftime(time(nullptr),worldTimer);
            if (difftime(time(nullptr),lastTimerCheck) > 30) {
                printf("%f seconds have passed.\n", gameDurationInSeconds);
                time(&lastTimerCheck);
            }
        }

        if (gameDurationInSeconds > GAME_DURATION && !gameOver) {
            printf("GAME OVER\n");
            gameOver = true;
            processGameOver();
        }
    }

    void World::processGameOver() {
        std::vector<PositionComponent>& positions = positionCM->getAllComponents();
        std::vector<PlayerDataComponent>& playerData = playerDataCM->getAllComponents();

        int teamBlueScore = playerData[0].points + playerData[1].points;
        int teamRedScore = playerData[2].points + playerData[3].points;

        if (teamBlueScore > teamRedScore) {
            // Winner at the foot of the bear
            positions[0].position = WINNER_1_POS;
            positions[1].position = WINNER_2_POS;

            // Losers to the side of the bear, clapping?
            positions[2].position = LOSER_1_POS;
            positions[3].position = LOSER_2_POS;

            winner = BLUE;
        } else if (teamRedScore > teamBlueScore) {
            positions[0].position = LOSER_1_POS;
            positions[1].position = LOSER_2_POS;

            positions[2].position = WINNER_1_POS;
            positions[3].position = WINNER_2_POS;

            winner = RED;
        }
        // What to do in case of tie?

    }

    void World::printDebug() {
    }

    void World::updatePlayerInput(unsigned int player, float pitch, float yaw, bool forwardRequested, bool backwardRequested, bool leftRequested, bool rightRequested, bool jumpRequested, bool throwEggRequested, bool shootRequested, bool abilityRequested, bool resetRequested) {
        MovementRequestComponent& req = movementRequestCM->lookup(players[player]);

        req.pitch = pitch;
        req.yaw = yaw;
        req.forwardRequested = forwardRequested;
        req.backwardRequested = backwardRequested;
        req.leftRequested = leftRequested;
        req.rightRequested = rightRequested;
        req.jumpRequested = jumpRequested;
        req.shootRequested = shootRequested;
        req.abilityRequested = abilityRequested;
        req.throwEggRequested = throwEggRequested;
        req.resetRequested = resetRequested;
    }
    
    void World::updatePlayerCharacterSelection(unsigned int player, int browsingCharacterUID, int characterUID) {
        charactersUID[player] = characterUID;
        browsingCharactersUID[player] = browsingCharacterUID;
    }

    Entity World::getFreshProjectile(BallProjType projType) {
        int i = 0;
        while (i < NUM_EACH_PROJECTILE) {
            Entity ballProjEntity = ballProjectiles[projType][i];
            BallProjDataComponent& data = ballProjDataCM->lookup(ballProjEntity);
            if (!data.active) {
                data.active = true;
                meshCollisionCM->lookup(ballProjEntity).active = true;
                return ballProjEntity;
            }
            i++;
        }
        // If we reach the end of this function, that means all projectiles are active and we don't have one to give
        assert(false);
        return NULL;
    }

    Entity World::getEgg() {
        return egg;
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
            packet.movementEntityStates[i][IS_SHOOTING] = requests[i].shootRequested;
            packet.movementEntityStates[i][IS_USING_ABILITY] = requests[i].abilityRequested;
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
        for (unsigned int i = 0; i < NUM_PROJ_TYPES; i++) {
            for (unsigned int j = 0; j < NUM_EACH_PROJECTILE; j++) {
                BallProjDataComponent& projData = ballProjDataCM->lookup(ballProjectiles[i][j]);
                packet.active[i * NUM_EACH_PROJECTILE + j] = projData.active;
                packet.movementEntityStates[projIndices[i * NUM_EACH_PROJECTILE + j]][EXPLODING] = projData.exploded;
            }
        }
        std::vector<HealthComponent> healths = healthCM->getAllComponents();
        std::vector<PlayerDataComponent> playerData = playerDataCM->getAllComponents();
        for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
            packet.healths[i] = healths[i].healthPoint;
            packet.scores[i] = playerData[i].points;
        }
        packet.currentSeason = currentSeason;
        packet.seasonBlend = ((float)seasonCounter) / SEASON_LENGTH;
    }

    void World::fillInBulletData(BulletPacket& packet) {
        // tell client about the bullets that spawned during this game tick
        for (int i = 0; i < bulletTrails.size(); i++) {
            packet.bulletTrail[i] = bulletTrails[i];
        }
        packet.count = bulletTrails.size();
        bulletTrails.clear();
    }

    void World::fillinGameEndData(GameEndPacket& packet) {
        packet.gameOver = gameOver;
        packet.winner = winner;
    }

    void World::fillInCharacterSelectionData(LobbyServerToClientPacket& packet) {
        for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
            packet.playersCharacter[i] = charactersUID[i];
            packet.playersBrowsingCharacter[i] = browsingCharactersUID[i];
            packet.teams[i] = teammates[i];
        }
    }


    bool World::withinMapBounds(glm::vec3 pos) {
        return pos.x >= minMapXValue && pos.x <= maxMapXValue && pos.y >= minMapYValue && pos.y <= maxMapYValue + HEIGHT_LIMIT && pos.z >= minMapZValue && pos.z <= maxMapZValue;
    }
} 