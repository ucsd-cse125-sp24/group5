#include "bge/World.h"

namespace bge {

    void World::init() {
        // First entity will get index 0
        currMaxEntityId = 0;

        initMesh();

        positionCM = std::make_shared<ComponentManager<PositionComponent>>();
        velocityCM = std::make_shared<ComponentManager<VelocityComponent>>();
        movementRequestCM = std::make_shared<ComponentManager<MovementRequestComponent>>();
        jumpInfoCM = std::make_shared<ComponentManager<JumpInfoComponent>>();
        std::shared_ptr<PlayerAccelerationSystem> playerAccSystem = std::make_shared<PlayerAccelerationSystem>(this, positionCM, velocityCM, movementRequestCM, jumpInfoCM);
        std::shared_ptr<MovementSystem> movementSystem = std::make_shared<MovementSystem>(this, positionCM, velocityCM);
        std::shared_ptr<CollisionSystem> collisionSystem = std::make_shared<CollisionSystem>(positionCM, velocityCM, jumpInfoCM);
        for (int i = 0; i < NUM_PLAYER_ENTITIES; i++) {
            Entity newPlayer = createEntity();
            players[i] = newPlayer;

            // Create components
            PositionComponent pos = PositionComponent(i*10.0f, 10.0f, -(i%2)*8.0f);
            addComponent(newPlayer, pos);

            VelocityComponent vel = VelocityComponent(0.0f, 0.0f, 0.0f);
            addComponent(newPlayer, vel);
            MovementRequestComponent req = MovementRequestComponent(false, false, false, false, false, 0, 0);
            addComponent(newPlayer, req);
            JumpInfoComponent jump = JumpInfoComponent(0, false);
            addComponent(newPlayer, jump);

            // Add to systems
            playerAccSystem->registerEntity(newPlayer);
            movementSystem->registerEntity(newPlayer);
            collisionSystem->registerEntity(newPlayer);
        }
        systems.push_back(playerAccSystem);
        systems.push_back(movementSystem);
        systems.push_back(collisionSystem);
    }

    rayIntersection World::intersect(glm::vec3 p0, glm::vec3 p1, float maxT) {
        rayIntersection bestIntersection;
        bestIntersection.t=INFINITY;
        for(int i=0; i<mapTriangles.size()/3; i++) {
            // glm::mat4 inv=glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::vec3 A=mapVertices[mapTriangles[3*i+0]];
            glm::vec3 B=mapVertices[mapTriangles[3*i+1]];
            glm::vec3 C=mapVertices[mapTriangles[3*i+2]];
            glm::vec3 n=glm::normalize(glm::cross((C-A), (B-A)));
            float t=(glm::dot(A, n)-glm::dot(p0, n))/glm::dot(p1, n);
            if(t>-0.001&&t<bestIntersection.t&&t<maxT+0.001) {
                glm::vec3 iPos=p0+t*p1;
                float area=glm::length(glm::cross(B-A, C-B))/2;
                float alpha=glm::length(glm::cross(B-iPos, C-iPos)/2.0f)/area;
                float beta=glm::length(glm::cross(A-iPos, C-iPos)/2.0f)/area;
                float gamma=glm::length(glm::cross(B-iPos, A-iPos)/2.0f)/area;
                if(alpha>=-0.01&&beta>=-0.01&&gamma>=-0.01&&alpha+beta+gamma<=1.01) {
                    bestIntersection.t=t;
                    bestIntersection.normal=n;
                    bestIntersection.tri=i;
                }
            }
        }

        return bestIntersection;
    }

    void World::initMesh() {
        Assimp::Importer importer;
        std::string mapFilePath = "../client/models/map_1_test.obj";
        const aiScene* scene = importer.ReadFile(mapFilePath,
            ASSIMP_IMPORT_FLAGS);
        if (scene == nullptr) {
            std::cerr << "Unable to load 3d model from path " << mapFilePath << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Loaded environment model\n";

        // Load meshes into ModelComposite data structures
        for (unsigned int i = 0; i < 1; i++) {
            aiMesh& mesh = *scene->mMeshes[i];
            // load vertices
            for (unsigned int j = 0; j < mesh.mNumVertices; j++) {
                const aiVector3D& vertex = mesh.mVertices[j];
                mapVertices.push_back(glm::vec3(vertex[0], vertex[1], vertex[2]));
            }
            // load triangles
            for (unsigned int j = 0; j < mesh.mNumFaces; j++) {
                const aiFace& face = mesh.mFaces[j];
                mapTriangles.push_back(face.mIndices[0]);
                mapTriangles.push_back(face.mIndices[1]);
                mapTriangles.push_back(face.mIndices[2]);
            }
        }

        std::cout << "loaded vertices and triangles\n";
    }

    Entity World::createEntity() {
        Entity newEntity = Entity();
        newEntity.id = currMaxEntityId;
        currMaxEntityId++;
        entities.insert(newEntity);
        return newEntity;
    }

    void World::addComponent(Entity e, PositionComponent c) {
        positionCM->add(e, c);
    }
    void World::addComponent(Entity e, VelocityComponent c) {
        velocityCM->add(e, c);
    }
    void World::addComponent(Entity e, MovementRequestComponent c) {
        movementRequestCM->add(e, c);
    }
    void World::addComponent(Entity e, JumpInfoComponent c) {
        jumpInfoCM->add(e, c);
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
        // this needs to be a reference beause the elements in systems are unique_ptrs
        for (auto& s : systems) {
            s->update();
        }
        // PositionComponent playerPos=positionCM->lookup(players[0]);
        // MovementRequestComponent movementRequest=movementRequestCM->lookup(players[0]);
        // glm::vec3 cameraDirection;
        // cameraDirection.x = cos(glm::radians(movementRequest.yaw)) * cos(glm::radians(movementRequest.pitch));
        // cameraDirection.y = sin(glm::radians(movementRequest.pitch));
        // cameraDirection.z = sin(glm::radians(movementRequest.yaw)) * cos(glm::radians(movementRequest.pitch));
        // rayIntersection intersection=World::intersect(playerPos.position, cameraDirection, 100);
        // std::cout<<"Minimum intersection distance: "<<intersection.t<<std::endl;
        // std::cout<<"tri: "<<intersection.tri<<std::endl;
        // PositionComponent& player1Pos=positionCM->lookup(players[1]);
        // if(intersection.t<100) {
        //     player1Pos.position=playerPos.position+glm::normalize(cameraDirection)*intersection.t;
        // }
    }

    void World::printDebug() {
    }

    void World::updatePlayerInput(unsigned int player, float pitch, float yaw, bool forwardRequested, bool backwardRequested, bool leftRequested, bool rightRequested, bool jumpRequested) {
        MovementRequestComponent& req = movementRequestCM->lookup(players[player]);

        req.pitch = pitch;
        req.yaw = yaw;
        req.forwardRequested = forwardRequested;
        req.backwardRequested = backwardRequested;
        req.leftRequested = leftRequested;
        req.rightRequested = rightRequested;
        req.jumpRequested = jumpRequested;
    }

    void World::fillInGameData(ServerToClientPacket& packet) {
        std::vector<PositionComponent> positions = positionCM->getAllComponents();
        for (int i = 0; i < positions.size(); i++) {
            packet.positions[i] = positions[i].position;
        }
        std::vector<VelocityComponent> velocities = velocityCM->getAllComponents();
        for (int i = 0; i < velocities.size(); i++) {
            packet.velocities[i] = velocities[i].velocity;
        }
        std::vector<MovementRequestComponent> requests = movementRequestCM->getAllComponents();
        for (int i = 0; i < requests.size(); i++) {
            packet.pitches[i] = requests[i].pitch;
            packet.yaws[i] = requests[i].yaw;
        }
    }

} 