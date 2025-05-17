// Include standard headers
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>

#include "LuigiEngine/PhysicsUtils.hpp"
#include "LuigiEngine/SceneMesh.hpp"
#include "Mesh.hpp"
#include "glm/detail/type_mat.hpp"

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <random>
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

#include "common/shader.hpp"


#include "ECS.h"
#include "RenderSystem.hpp"
#include "SceneCamera.hpp"
#include "Transform.hpp"
#include "Physics.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#include "ImGui.hpp"
#include "ImGuiConsole.hpp"
#include "ImGuiHelper.hpp"
#include "SceneRenderer.hpp" // #include "SceneCamera.cpp"

#include "Constraint.hpp"
#include "Vehicle.hpp"

void processInput(GLFWwindow *window, float deltaTime, Registry & registry, RenderSystem & renderSystem);

// settings
constexpr int SCR_WIDTH = 1920;
constexpr int SCR_HEIGHT = 1080;

Registry registry;

// timing
double deltaTime; // time between current frame and last frame
double lastFrame;
double timeSinceKeyPressed = 0.0f;
int refreshrateMode = 1; // 1 = V-Sync  0 = inf
bool paused = false;

// debug
double startTime;
int nbFrames = 0;
bool* optimizeMVP = new bool(true);
int* nbLocalMatrixUpdate;
int* nbGlobalMatrixUpdate;
int* nbMVPUpdate;
int* nbViewProjUpdate;

// déplacement selon heightmap
unsigned char* heightmapData;
int heightmapWidth;
int heightmapHeight;
ivec2 terrainSize{10, 10};

int physicsPrecision = 2;
const float physicsStep = 1.0/60.0f; //delta constant pour la physique
float physicsFrameTime = 0.0f;
float timeScale = 1.0f;

/*******************************************************************************/

// TODO [TP01] Génération de terrain plat
void createFlatTerrain(ivec2 terrain_resolution, vec2 terrain_size, vector<vec3>& vertices, vector<unsigned int>& triangles, vector<vec2>& uvs)
{
    vertices.clear();
    uvs.clear();
    triangles.clear();

    for (int i = 0; i < terrain_resolution.y; ++i)
        for (int j = 0; j < terrain_resolution.x; ++j)
        {
            vertices.emplace_back(1.0*j / (terrain_resolution.x-1) * terrain_size.x, 0.0f, 1.0*i / (terrain_resolution.y-1) * terrain_size.y);
            uvs.emplace_back(1.0*j / (terrain_resolution.x-1), 1.0*i / (terrain_resolution.y-1));
        }

    for (int i = 0; i < terrain_resolution.y - 1; ++i) {
        for (int j = 0; j < terrain_resolution.x - 1; ++j) {
            int top_left = i * terrain_resolution.x + j;
            int top_right = top_left + 1;
            int bottom_left = (i + 1) * terrain_resolution.x + j;
            int bottom_right = bottom_left + 1;

            // Triangle 1 (top left, bottom left, top_right)
            triangles.emplace_back(top_left);
            triangles.emplace_back(bottom_left);
            triangles.emplace_back(top_right);

            // Triangle 2 (top_right, bottom_left, bottom_right)
            triangles.emplace_back(top_right);
            triangles.emplace_back(bottom_left);
            triangles.emplace_back(bottom_right);
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

    if (ImGui::GetIO().WantCaptureMouse) { // avec l'interface qu'on a, ce truc est toujours vrai
        //Console::getInstance().addLog("Imgui has mouse");
        //return;
    }else{
        //Console::getInstance().addLog("opengl has mouse");
    }

    static glm::vec2 lastMousePos = glm::vec2(0.0f, 0.0f);
    glm::vec2 currentMousePos(xpos, ypos);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        glm::vec2 offset = currentMousePos - lastMousePos;

        CameraComponent& camera = registry.get<CameraComponent>(0); 
        Transform& cameraTransform = registry.get<Transform>(0);

        offset *= 0.1f;
        const vec3 VEC_UP = {0.0f, 1.0f, 0.0f};
        mat4  rotationMatrix = cameraTransform.getRot();
        vec3 right = vec3(rotationMatrix * vec4(1.0f, 0.0f, 0.0f, 0.0f));
        glm::quat yaw = glm::angleAxis(glm::radians(-offset.x), VEC_UP);
        glm::quat pitch = glm::angleAxis(glm::radians(-offset.y), right);

        mat4 quaternionMatrix = glm::mat4_cast(glm::normalize(yaw * pitch));
        cameraTransform.setRot(quaternionMatrix * rotationMatrix);
    }

    lastMousePos = currentMousePos;
}

GLuint simpleShaders;

//for testing
Mesh* testMesh;
MeshComponent testMeshComponent;
TextureComponent testTextureComponent;
Collider* testCollider;

Entity debugCube1, debugCube2, debugCube3, debugCube4, debugCube5, debugCube6;

SphereCollider* sphereCollider;
MeshComponent sphereMeshComponent;

ConstraintSystem constraintSystem;

static std::mt19937 rng(std::random_device{}());
static std::uniform_int_distribution<int> distName(0, 9999);

int cubeCount;

void makeBox(Registry& registry, vec3 position, float scale = 1.0f ){
    
    std::string randomName = "Object" + std::to_string(cubeCount++);
    Entity object = registry.create();

    Transform& transform = registry.emplace<Transform>(object);
    transform.setPos(position);
    transform.setScale(vec3(scale));

    registry.emplace<TextureComponent>(object, testTextureComponent);
    registry.emplace<MeshComponent>(object, testMeshComponent);
    registry.emplace<Hierarchy>(object, std::vector<Entity>{}).name = randomName;
    auto& rigidBody = registry.emplace<RigidBodyComponent>(object);

    rigidBody.addCollider(testCollider);
}

void makeFloor(Registry& registry, int tilesX = 10, int tilesZ = 10, float tileSize = 2.0f, float y = -5.0f) {
    for (int x = 0; x < tilesX; ++x) {
        for (int z = 0; z < tilesZ; ++z) {
            Entity tile = registry.create();

            Transform& transform = registry.emplace<Transform>(tile);
            transform.setPos(vec3((x - tilesX / 2) * tileSize *2+ tileSize , y, (z - tilesZ / 2) * tileSize*2 + tileSize ));
            transform.setScale(vec3(tileSize, 0.5f, tileSize));

            registry.emplace<TextureComponent>(tile, testTextureComponent);
            registry.emplace<MeshComponent>(tile, testMeshComponent);
            registry.emplace<Hierarchy>(tile, std::vector<Entity>{}).name = "FloorTile_" + std::to_string(x) + "_" + std::to_string(z);

            auto& body = registry.emplace<RigidBodyComponent>(tile);
            body.bodyType = PhysicsType::STATIC;
            body.addCollider(new OBBCollider(vec3(1.0f,1.0f,1.0f)));
            body.mass = 100.0;
            body.inverseMass = 1.0 / body.mass;
        }
    }
}

void spawnRandom(Registry& registry, vec3 position){

    std::uniform_real_distribution<float> distPos(-10.0f, 10.0f);
    std::uniform_real_distribution<float> distScale(0.5f, 2.0f);
    std::uniform_int_distribution<int> distShape(0, 2); // 0: cube, 1: sphere, 2: cylinder

    vec3 randomPosition(distPos(rng), distPos(rng), distPos(rng));
    float randomScale = distScale(rng);

    int shapeType = distShape(rng);
    if (shapeType == 0) {
        makeBox(registry, randomPosition, randomScale);
    } else if (shapeType == 1) {
        std::string randomName = "Sphere" + std::to_string(cubeCount++);
        Entity sphere = registry.create();

        Transform& transform = registry.emplace<Transform>(sphere);
        transform.setPos(randomPosition);
        transform.setScale(vec3(randomScale));

        registry.emplace<TextureComponent>(sphere, testTextureComponent);
        registry.emplace<MeshComponent>(sphere, sphereMeshComponent);
        registry.emplace<Hierarchy>(sphere, std::vector<Entity>{}).name = randomName;
        auto& rigidBody = registry.emplace<RigidBodyComponent>(sphere);

        rigidBody.addCollider(sphereCollider);
    } else if (shapeType == 2) {
        std::string randomName = "Cylinder" + std::to_string(cubeCount++);
        Entity cylinder = registry.create();

        Transform& transform = registry.emplace<Transform>(cylinder);
        transform.setPos(randomPosition);
        transform.setScale(vec3(randomScale));

        registry.emplace<TextureComponent>(cylinder, testTextureComponent);
        registry.emplace<MeshComponent>(cylinder, MeshComponent({{0, new Mesh("models/cylinder.obj")}}, simpleShaders));
        registry.emplace<Hierarchy>(cylinder, std::vector<Entity>{}).name = randomName;
        auto& rigidBody = registry.emplace<RigidBodyComponent>(cylinder);

        rigidBody.addCollider(new CylinderCollider(0.95f, 1.4f));
    }

}


int main()
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "TP Moteur - GLFW", nullptr, nullptr);
    if( window == nullptr ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, SCR_WIDTH/2.0, SCR_HEIGHT/2.0);

    // Dark blue background
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

    glfwSetCursorPosCallback(window, mouse_callback);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    // glEnable(GL_CULL_FACE);

    glEnable(GL_TEXTURE_2D);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    Console& console = Console::getInstance();

    /****************************************/
    //ECS setup


    TransformSystem transformSystem;
    RenderSystem renderSystem = RenderSystem();
    CameraSystem cameraSystem = CameraSystem();
    PhysicsSystem physicsSystem = PhysicsSystem();
    constraintSystem = ConstraintSystem();
    VehicleSystem vehicleSystem = VehicleSystem();

    

    //shaders setup

    simpleShaders  = LoadShaders("shaders/vertex.glsl", "shaders/fragment.glsl");
    GLuint phongShaders = LoadShaders("shaders/vertex_phong.glsl", "shaders/fragment_phong.glsl");
    GLuint terrainShaders = LoadShaders("shaders/vertex_terrain.glsl", "shaders/fragment_terrain.glsl");


    //Meshes setup
    
    Mesh* sphereLOD1 = new Mesh("models/sphereLOD1.obj");
    Mesh* sphereLOD2 = new Mesh("models/sphereLOD2.obj");

    Mesh* suzanneLOD1 = new Mesh("models/suzanneLOD1.obj");
    Mesh* suzanneLOD2 = new Mesh("models/suzanneLOD2.obj");

    Mesh* terrainMesh = new Mesh();
    createFlatTerrain(glm::vec2(2,2), glm::vec2(20,20), terrainMesh->vertices, terrainMesh->triangles, terrainMesh->uvs);

    Mesh* cubeMesh = new Mesh("models/cube.obj");

    Mesh* cylinderMesh = new Mesh("models/cylinder.obj");

    Mesh* raceCarMesh = new Mesh("models/vehicle-racer.obj");

    Mesh* wheelMesh = new Mesh("models/wheel-medium.obj");
    //makeCubeMesh(cubeMesh->vertices, cubeMesh->normals, cubeMesh->uvs, cubeMesh->triangles);

    /* quand on cree une mesh attache automatiquement un TextureComponent
    TextureComponent textureComponent;
    textureComponent.texFiles.push_back("sun.jpg");
    textureComponent.texUniforms.push_back("tex"); */

    OBBCollider* cubeCollider = new OBBCollider(vec3(1.0f, 1.0f, 1.0f));

    sphereCollider = new SphereCollider(1.0f);
    sphereCollider->mass = 100.0f;
    sphereCollider->computeInertiaTensor();
    
    PlaneCollider* planeCollider = new PlaneCollider(vec3(0.0f,1.0f,0.0f));

    CylinderCollider* cylinderCollider = new CylinderCollider(0.95f,1.4f);

    //MeshComponent setup
    MeshComponent terrainMeshComponent = MeshComponent({{0,cubeMesh}}, simpleShaders, {"moon.jpg"}, {"tex"});

    MeshComponent cubeMeshComponent = MeshComponent({{0,cubeMesh}}, simpleShaders, {"sun.jpg"}, {"tex"});

    sphereMeshComponent = MeshComponent({{0,sphereLOD1}}, simpleShaders, {"venus.jpg"}, {"tex"});

    MeshComponent cylinderMeshComponent({{0, cylinderMesh}}, simpleShaders);

    MeshComponent suzanneMeshComponent({{0, suzanneLOD1}}, simpleShaders, {"suzanne.png"}, {"tex"});

    MeshComponent raceCarMeshComponent({{0, raceCarMesh}}, simpleShaders, {"Heightmap_Rocky.png"}, {"tex"});

    MeshComponent wheelMeshComponent({{0, wheelMesh}}, simpleShaders, {"rock.jpg"}, {"tex"});

    TextureComponent snowRockTextureComponent = TextureComponent({"snowrock.png"}, {"tex"});

     //testing
    testMesh = new Mesh("models/cube.obj");
    //testMesh = new Mesh("models/sphereLOD1.obj");
    testMeshComponent = MeshComponent({{0,testMesh}}, simpleShaders);
    testTextureComponent = TextureComponent({"moon.jpg"}, {"tex"});
    testCollider = new OBBCollider(vec3(1.0f, 1.0f, 1.0f));
    //testCollider = new SphereCollider(1.0f);

    SuspensionConstraint suspensionConstraint;

    Entity raceCarEntity;
    
    auto setupScene = [&](){
        registry.clear();
        constraintSystem.suspensionConstraints.clear();

        //camera setup
        Entity cameraWorldSideEntity = registry.create();
        mat4 pers = perspective(radians(45.0f), 1.0f * SCR_WIDTH / SCR_HEIGHT, 0.1f, 1000.0f);
        registry.emplace<Transform>(cameraWorldSideEntity).addPos({0, 0, 20});
        registry.emplace<CameraComponent>(cameraWorldSideEntity, pers).speed = 10.0f;
        renderSystem.activeCamera = cameraWorldSideEntity;
        
        //entities create and attach comps

        Entity terrainEntity = registry.create(); 

        registry.emplace<Transform>(terrainEntity).setPos(vec3(0,-5,0));
        registry.get<Transform>(terrainEntity).setScale(vec3(20,0.5,20));
        //registry.get<Transform>(terrainEntity).setRot(glm::rotate(mat4(1.0f), glm::radians(-10.0f), vec3(0.0f, 0.0f, 1.0f)));
        registry.emplace<MeshComponent>(terrainEntity, terrainMeshComponent);
        registry.emplace<Hierarchy>(terrainEntity, vector<Entity>{}).name = "Terrain";
        auto& body = registry.emplace<RigidBodyComponent>(terrainEntity);
        body.bodyType = PhysicsType::STATIC;
        body.addCollider(cubeCollider);

        {
            Entity slopeEntity = registry.create();

            Transform& slopeTransform = registry.emplace<Transform>(slopeEntity);
            slopeTransform.setPos(vec3(12, -2, 0));
            slopeTransform.setScale(vec3(8, 0.5, 8));
            slopeTransform.setRot(glm::rotate(mat4(1.0f), glm::radians(30.0f), vec3(0.0f, 0.0f, 1.0f)));

            registry.emplace<MeshComponent>(slopeEntity, testMeshComponent);
            registry.emplace<Hierarchy>(slopeEntity, std::vector<Entity>{}).name = "Slope";
            auto& slopeBody = registry.emplace<RigidBodyComponent>(slopeEntity);
            slopeBody.bodyType = PhysicsType::STATIC;
            slopeBody.addCollider(new OBBCollider(vec3(1.0f, 1.0f, 1.0f)));
            slopeBody.mass = 100.0;
            slopeBody.inverseMass = 1.0 / slopeBody.mass;
        }
  

        //makeFloor(registry, 5, 5, 2.0f);

        /* Entity terrainEntity = registry.create(); 

        registry.emplace<Transform>(terrainEntity).setPos(vec3(0,-60,0));
        registry.get<Transform>(terrainEntity).setScale(vec3(60));
        registry.emplace<MeshComponent>(terrainEntity, sphereMeshComponent);
        registry.emplace<Hierarchy>(terrainEntity, vector<Entity>{}).name = "Terrain";
        auto& body = registry.emplace<RigidBodyComponent>(terrainEntity);
        body.bodyType = PhysicsType::STATIC;
        body.addCollider(sphereCollider);
        body.mass = 1000.0;
        body.inverseMass = 1.0/body.mass; */

        /* Entity downBorderEntity = registry.create();

        registry.emplace<Transform>(downBorderEntity).setPos(vec3(0,-10,0));
        registry.emplace<Hierarchy>(downBorderEntity, vector<Entity>{}).name = "Border";
        auto& body2 = registry.emplace<RigidBodyComponent>(downBorderEntity);
        body2.bodyType = PhysicsType::STATIC;
        body2.addCollider(planeCollider); */

        /* Entity cylinderEntity = registry.create();

        registry.emplace<Transform>(cylinderEntity).setPos(vec3(0, 0, 0));
        registry.get<Transform>(cylinderEntity).setScale(vec3(1.0f, 0.5f, 1.0f));
        registry.get<Transform>(cylinderEntity).setRot(glm::rotate(mat4(1.0f), glm::half_pi<float>() , vec3(0.0f, 0.0f, 1.0f)));
        registry.emplace<TextureComponent>(cylinderEntity,snowRockTextureComponent);
        registry.emplace<MeshComponent>(cylinderEntity, cylinderMeshComponent);
        registry.emplace<Hierarchy>(cylinderEntity, vector<Entity>{}).name = "Cylinder";
        auto& cylinderBody = registry.emplace<RigidBodyComponent>(cylinderEntity);
        cylinderBody.addCollider(cylinderCollider); */

        
        /* Entity sphere1Entity = registry.create();
        registry.emplace<Transform>(sphere1Entity).setPos(vec3(-3,3,0));
        registry.emplace<MeshComponent>(sphere1Entity, sphereMeshComponent);
        registry.emplace<Hierarchy>(sphere1Entity, vector<Entity>{}).name = "Sphere1";
        registry.emplace<RigidBodyComponent>(sphere1Entity).addCollider(sphereCollider); */

        {
            debugCube1 = registry.create();
            Transform& transform1 = registry.emplace<Transform>(debugCube1);
            transform1.setPos(vec3(-2.0f, 0.0f, -2.0f));
            transform1.setScale(vec3(0.1f));
            //registry.emplace<TextureComponent>(debugCube1, moonTextureComponent);
            registry.emplace<MeshComponent>(debugCube1, sphereMeshComponent);
            registry.emplace<Hierarchy>(debugCube1, std::vector<Entity>{}).name = "DebugCube1";
        }

        {
            debugCube2 = registry.create();
            Transform& transform2 = registry.emplace<Transform>(debugCube2);
            transform2.setPos(vec3(2.0f, 0.0f, 2.0f));
            transform2.setScale(vec3(0.1f));
            //registry.emplace<TextureComponent>(debugCube2, mercuryTextureComponent);
            registry.emplace<MeshComponent>(debugCube2, cubeMeshComponent);
            registry.emplace<Hierarchy>(debugCube2, std::vector<Entity>{}).name = "DebugCube2";
        } 

        {
            debugCube3 = registry.create();
            Transform& transform3 = registry.emplace<Transform>(debugCube3);
            transform3.setPos(vec3(0.0f, 0.0f, 0.0f));
            transform3.setScale(vec3(0.1f));
            registry.emplace<MeshComponent>(debugCube3, sphereMeshComponent);
            registry.emplace<Hierarchy>(debugCube3, std::vector<Entity>{}).name = "DebugCube3";
        }

        {
            debugCube4 = registry.create();
            Transform& transform4 = registry.emplace<Transform>(debugCube4);
            transform4.setPos(vec3(0.0f, 0.0f, 0.0f));
            transform4.setScale(vec3(0.1f));
            registry.emplace<MeshComponent>(debugCube4, cubeMeshComponent);
            registry.emplace<Hierarchy>(debugCube4, std::vector<Entity>{}).name = "DebugCube4";
        }
        {
            debugCube5 = registry.create();
            Transform& transform5 = registry.emplace<Transform>(debugCube5);
            transform5.setPos(vec3(0.0f, 0.0f, 0.0f));
            transform5.setScale(vec3(0.25f));
            registry.emplace<MeshComponent>(debugCube5,suzanneMeshComponent );
            registry.emplace<Hierarchy>(debugCube5, std::vector<Entity>{}).name = "DebugCube5";
        }

         /* Entity sphere2Entity = registry.create();
        registry.emplace<Transform>(sphere2Entity).setPos(vec3(-3.5,6,3));
        registry.emplace<MeshComponent>(sphere2Entity, sphereMeshComponent);
        registry.emplace<Hierarchy>(sphere2Entity, vector<Entity>{}).name = "Sphere2";
        registry.emplace<RigidBodyComponent>(sphere2Entity).addCollider(sphereCollider); */
        
        
        
        /* Entity cube1Entity = registry.create();
        registry.emplace<Transform>(cube1Entity).setPos(vec3(10,0,0));
        registry.get<Transform>(cube1Entity).setScale(vec3(3.0f,3.0f,3.5f));
        registry.emplace<MeshComponent>(cube1Entity, raceCarMeshComponent);
        registry.emplace<Hierarchy>(cube1Entity, vector<Entity>{}).name = "Cube1";
        registry.emplace<RigidBodyComponent>(cube1Entity).addCollider(cubeCollider);

        
        suspensionConstraint.entity = cube1Entity;
        suspensionConstraint.localAnchor = vec3(0.0f, 0.0f, 0.0f);
        suspensionConstraint.direction = vec3(0.0f, -1.0f, 0.0f);
        suspensionConstraint.initialLength = 1.0f;
        suspensionConstraint.stiffness = 10000.0f;
        suspensionConstraint.damping = 1000.0f;

        constraintSystem.suspensionConstraints.push_back(suspensionConstraint); */

        raceCarEntity = registry.create();
        VehicleComponent& vehicle = registry.emplace<VehicleComponent>(raceCarEntity);
        vehicle.setup(registry, constraintSystem, raceCarMeshComponent, wheelMeshComponent, vec3(0.0f, 5.0f, 0.0f), vec3(3.0f), 2.0f, 2.0f);

        Transform& raceCarTransform = registry.get<Transform>(raceCarEntity);
        float randomAngle = std::uniform_real_distribution<float>(0.0f, glm::two_pi<float>())(rng);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), randomAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        raceCarTransform.setRot(rotation * raceCarTransform.getRot());

        //raceCar = Vehicle(registry, constraintSystem, raceCarMeshComponent, wheelMeshComponent, vec3(3.0f, 5.0f, 3.0f), vec3(3.0f), 1.0f, 1.0f);


        std::cout << "creating boxes " << std::endl;

        int size = 5;
        double totalTime = 0.0;
        int totalBoxes = size * size * size;
        cubeCount = 0;
        float scale = 0.5f;

        for (int x = 0; x < size; ++x) {
            for (int y = 0; y < size; ++y) {
                for (int z = 0; z < size; ++z) {
                    double startTime = glfwGetTime();
                    //makeBox(registry, vec3(x * scale * 2, y * scale * 2, z * scale *2), scale);
                    //spawnRandom(registry, vec3(x * scale * 2, y * scale * 2, z * scale *2));
                    double endTime = glfwGetTime();
                    double boxTime = endTime - startTime;
                    totalTime += boxTime;
                }
            }
        }

        double averageTime = totalTime / totalBoxes;
        console.addLog("Average time to create a box: " + to_string(averageTime) + " seconds");
    };
    

    setupScene();
    

   

    //main loop
 
    

    initImGui(window);
    SceneRenderer& sceneRenderer = SceneRenderer::getInstance();
    if (!sceneRenderer.setupFramebuffer(SCR_WIDTH, SCR_HEIGHT, 1.0f)) {
        console.addLog("Failed to initialize Scene Renderer");
    }

 
    lastFrame = glfwGetTime();

    

    do {
        // Measure speed
        // per-frame time logic
        const double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        timeSinceKeyPressed += deltaTime;
        if (!nbFrames)
            startTime = glfwGetTime();
        nbFrames++;

        // Inputs
        processInput(window, deltaTime, registry, renderSystem);

        // Debug
        nbLocalMatrixUpdate = new int(0);
        nbGlobalMatrixUpdate = new int(0);
        nbMVPUpdate = new int(0);
        nbViewProjUpdate = new int(0);

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        cameraSystem.update(registry);
        cameraSystem.computeViewProj(registry);


        if (!paused) {
            double physicsStartTime = glfwGetTime();
            for (int i = 0; i < physicsPrecision; ++i) {
                transformSystem.update(registry); 
                physicsSystem.update(registry, (physicsStep * timeScale / (float) physicsPrecision));
                constraintSystem.update(registry, (physicsStep * timeScale / (float) physicsPrecision));
                
            }
            double physicsEndTime = glfwGetTime();
            double physicsDuration = physicsEndTime - physicsStartTime;
            physicsFrameTime = physicsDuration;
            for(CollisionInfo& info : physicsSystem.getCollisionList()){

                Transform& transformA = registry.get<Transform>(info.entityA);
                Transform& transformB = registry.get<Transform>(info.entityB);

                RigidBodyComponent& rigidBodyA = registry.get<RigidBodyComponent>(info.entityA);
                RigidBodyComponent& rigidBodyB = registry.get<RigidBodyComponent>(info.entityB);

                //registry.get<Transform>(debugCube1).setPos(info.collisionPointA);

                //registry.get<Transform>(debugCube2).setPos(info.collisionPointB);
                

                /* vector<vec3> colPoints = info.collisionPoints;
                int size = colPoints.size();
                //std::cout << size << std::endl;
                if (size >= 1) registry.get<Transform>(debugCube1).setPos(colPoints[0]);
                if (size >= 2) registry.get<Transform>(debugCube2).setPos(colPoints[1]);
                if (size >= 3) registry.get<Transform>(debugCube3).setPos(colPoints[2]);
                if (size >= 4) registry.get<Transform>(debugCube4).setPos(colPoints[3]);

                glm::vec3 avgPos = (
                    registry.get<Transform>(debugCube1).getPos() +
                    registry.get<Transform>(debugCube2).getPos() +
                    registry.get<Transform>(debugCube3).getPos() +
                    registry.get<Transform>(debugCube4).getPos()
                ) / 4.0f;
                registry.get<Transform>(debugCube5).setPos(avgPos); */
            } 
        } else {
            transformSystem.update(registry); 
        }
        

       

        vehicleSystem.update(registry, (physicsStep * timeScale / (float) physicsPrecision));

        VehicleComponent& vehicle = registry.get<VehicleComponent>(raceCarEntity);
        //debug
        registry.get<Transform>(debugCube1).setPos(vehicle.FLSuspension->endPointWorld);
        registry.get<Transform>(debugCube2).setPos(vehicle.FRSuspension->endPointWorld);
        registry.get<Transform>(debugCube3).setPos(vehicle.BLSuspension->endPointWorld);
        registry.get<Transform>(debugCube4).setPos(vehicle.BRSuspension->endPointWorld);
        
        if(vehicle.FLSuspension->isColliding){
            registry.get<Transform>(debugCube1).setPos(vehicle.FLSuspension->collisionPoint);
        }
        if(vehicle.FRSuspension->isColliding){
            registry.get<Transform>(debugCube2).setPos(vehicle.FRSuspension->collisionPoint);
        }
        if(vehicle.BLSuspension->isColliding){
            registry.get<Transform>(debugCube3).setPos(vehicle.BLSuspension->collisionPoint);
        }
        if(vehicle.BRSuspension->isColliding){
            registry.get<Transform>(debugCube4).setPos(vehicle.BRSuspension->collisionPoint);
        }



        

        if (sceneRenderer.isInitialized()) {
            if(!sceneRenderer.render(deltaTime, paused, renderSystem, registry)){
                console.addLog("Scene Renderer error");
            }

        }

        //console.addLog("Physics updates: " + to_string(physicsUpdate));
        //physicsUpdate = 0;

        //reset la scene
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) setupScene();

        

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderImGui(registry);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwSwapInterval(refreshrateMode);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader

    delete optimizeMVP;
    delete nbLocalMatrixUpdate;
    delete nbGlobalMatrixUpdate;
    delete nbMVPUpdate;
    delete nbViewProjUpdate;

    delete sphereLOD1;
    delete sphereLOD2;
    delete suzanneLOD1;
    delete suzanneLOD2;
    delete terrainMesh;
    delete cubeMesh;
    delete testMesh;

    delete cubeCollider;

    delete testCollider;

    if (heightmapData) {
        stbi_image_free(heightmapData);
    }

    registry.clear();
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window, float deltatime, Registry & registry, RenderSystem & renderSystem)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera movement
    CameraComponent & camera = registry.get<CameraComponent>(renderSystem.activeCamera);
    Transform & cameraTransform = registry.get<Transform>(renderSystem.activeCamera);
    
    float velocity = camera.speed * deltatime;
    mat4 rotationMatrix = cameraTransform.getRot();
    vec3 front = vec3(rotationMatrix * vec4(0.0f, 0.0f, -1.0f, 0.0f));
    vec3 right = vec3(rotationMatrix * vec4(1.0f, 0.0f, 0.0f, 0.0f));
    vec3 up = vec3(rotationMatrix * vec4(0.0f, 1.0f, 0.0f, 0.0f));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraTransform.addPos(front * velocity);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraTransform.addPos(-front * velocity);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraTransform.addPos(-right * velocity);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraTransform.addPos(right * velocity);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraTransform.addPos(up * velocity);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraTransform.addPos(-up * velocity);


    // Debug
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && timeSinceKeyPressed >= 1.0f) {
        cout << "----- DEBUG -----" << endl;
        cout << "Frame Drawn : " << nbFrames << endl;
        cout << "Time : " << glfwGetTime() - startTime << endl;
        cout << "FPS : " << nbFrames/(glfwGetTime() - startTime) << endl;
        cout << "Delta time : " << deltaTime * 1000 << endl;
        cout << "----- Calcul de matrice par frame (OPTI: " << *optimizeMVP << ") -----" << endl;
        cout << *nbLocalMatrixUpdate << " calculs de matrice modèle LOCALE" << endl;
        cout << *nbGlobalMatrixUpdate << " calculs de matrice modèle GLOBALE" << endl;
        cout << *nbViewProjUpdate << " calculs de matrice VIEW-PROJECTION" << endl;
        cout << *nbMVPUpdate << " calculs de matrice MVP" << endl;
        cout << "----------" << endl;
        timeSinceKeyPressed = 0.0;
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && timeSinceKeyPressed >= 1.0f) {
        optimizeMVP = new bool(!*optimizeMVP);
        cout << "Switched MVP optimization." << endl;
        timeSinceKeyPressed = 0.0;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && timeSinceKeyPressed >= 1.0f) {
        paused = !paused;
        timeSinceKeyPressed = 0;
    }

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && timeSinceKeyPressed >= 1.0f) {
        if (refreshrateMode) {
            refreshrateMode = 0;
            cout << "Refresh rate mode set to infinite." << endl;
        } else {
            refreshrateMode = 1;
            cout << "Refresh rate mode set to V-Sync." << endl;
        }
        nbFrames = 0;
        timeSinceKeyPressed = 0.0;
    }

    if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && timeSinceKeyPressed >= 0.01f){

        static std::mt19937 rng(std::random_device{}());
        static std::uniform_real_distribution<float> distPos(-40.0f, 40.0f);
        static std::uniform_real_distribution<float> distAngle(0.0f, glm::two_pi<float>());
        static std::uniform_int_distribution<int> distName(0, 999);

        for (int i = 0; i < 1; ++i) {
            float randomX = distPos(rng);
            float randomY = distPos(rng);
            std::string randomName = "Cube" + std::to_string(distName(rng));

            Entity cube = registry.create();

            Transform& transform = registry.emplace<Transform>(cube);
            transform.setPos(glm::vec3(randomX, 0.0f, randomY));

            glm::quat rotation = glm::angleAxis(distAngle(rng), glm::vec3(1, 0, 0)) *
                                glm::angleAxis(distAngle(rng), glm::vec3(0, 1, 0)) *
                                glm::angleAxis(distAngle(rng), glm::vec3(0, 0, 1));
            transform.setRot(glm::toMat4(rotation));

            registry.emplace<MeshComponent>(cube, testMeshComponent);
            registry.emplace<Hierarchy>(cube, std::vector<Entity>{}).name = randomName;

            auto& rigidBody = registry.emplace<RigidBodyComponent>(cube);
            rigidBody.mesh = testMesh;
            rigidBody.addCollider(testCollider);
        }

        timeSinceKeyPressed = 0.0;
    }
}



// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
