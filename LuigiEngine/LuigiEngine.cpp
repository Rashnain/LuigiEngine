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
OBBCollider* testOBBCollider;

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

    /****************************************/
    //ECS setup


    TransformSystem transformSystem;
    RenderSystem renderSystem = RenderSystem();
    CameraSystem cameraSystem = CameraSystem();
    PhysicsSystem physicsSystem = PhysicsSystem();

    //camera setup
    Entity cameraWorldSideEntity = registry.create();
    mat4 pers = perspective(radians(45.0f), 1.0f * SCR_WIDTH / SCR_HEIGHT, 0.1f, 1000.0f);
    registry.emplace<Transform>(cameraWorldSideEntity).addPos({0, 0, 20});
    registry.emplace<CameraComponent>(cameraWorldSideEntity, pers).speed = 10.0f;
    renderSystem.activeCamera = cameraWorldSideEntity;

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
    //makeCubeMesh(cubeMesh->vertices, cubeMesh->normals, cubeMesh->uvs, cubeMesh->triangles);

    /* quand on cree une mesh attache automatiquement un TextureComponent
    TextureComponent textureComponent;
    textureComponent.texFiles.push_back("sun.jpg");
    textureComponent.texUniforms.push_back("tex"); */

    OBBCollider* cubeCollider = new OBBCollider(vec3(1.0f, 1.0f, 1.0f));

    SphereCollider* sphereCollider = new SphereCollider(1.0f);
    
    PlaneCollider* planeCollider = new PlaneCollider(vec3(0.0f,1.0f,0.0f));

    
    

    //MeshComponent setup
    MeshComponent terrainMeshComponent = MeshComponent({{0,cubeMesh}}, simpleShaders, {"moon.jpg"}, {"tex"});

    MeshComponent cubeMeshComponent = MeshComponent({{0,cubeMesh}}, simpleShaders, {"sun.jpg"}, {"tex"});

    MeshComponent sphereMeshComponent = MeshComponent({{0,sphereLOD1}}, simpleShaders, {"mercury.jpg"}, {"tex"});
    
    //entities create and attach comps

    Entity terrainEntity = registry.create(); 

    registry.emplace<Transform>(terrainEntity).setPos(vec3(0,-5,0));
    registry.get<Transform>(terrainEntity).setScale(vec3(40,0.5,40));
    registry.emplace<MeshComponent>(terrainEntity, terrainMeshComponent);
    registry.emplace<Hierarchy>(terrainEntity, vector<Entity>{}).name = "Terrain";
    auto& body = registry.emplace<RigidBodyComponent>(terrainEntity);
    body.bodyType = PhysicsType::STATIC;
    body.mesh = cubeMesh;
    body.colliders.push_back(cubeCollider);


    

    Entity sphere1Entity = registry.create();
    registry.emplace<Transform>(sphere1Entity).setPos(vec3(-5,0,0));
    registry.emplace<MeshComponent>(sphere1Entity, sphereMeshComponent);
    registry.emplace<Hierarchy>(sphere1Entity, vector<Entity>{}).name = "Sphere1";
    registry.emplace<RigidBodyComponent>(sphere1Entity).mesh = sphereLOD1;
    registry.get<RigidBodyComponent>(sphere1Entity).colliders.push_back(sphereCollider);
    registry.get<RigidBodyComponent>(sphere1Entity).linearVelocity = vec3(3,0,0);

    Entity cube1Entity = registry.create();
    registry.emplace<Transform>(cube1Entity).setPos(vec3(5,0,0));
    registry.emplace<MeshComponent>(cube1Entity, cubeMeshComponent);
    registry.emplace<Hierarchy>(cube1Entity, vector<Entity>{}).name = "Cube1";
    registry.emplace<RigidBodyComponent>(cube1Entity).mesh = cubeMesh;
    registry.get<RigidBodyComponent>(cube1Entity).colliders.push_back(cubeCollider);

    

    //testing
    testMesh = new Mesh("models/cube.obj");
    testMeshComponent = MeshComponent({{0,testMesh}}, simpleShaders, {"sun.jpg"}, {"tex"});
    testOBBCollider = new OBBCollider(vec3(1.0f, 1.0f, 1.0f));

    //main loop
 
    Console& console = Console::getInstance();

    initImGui(window);
    SceneRenderer& sceneRenderer = SceneRenderer::getInstance();
    if (!sceneRenderer.setupFramebuffer(SCR_WIDTH, SCR_HEIGHT, 1.0f)) {
        console.addLog("Failed to initialize Scene Renderer");
    }

 
    lastFrame = glfwGetTime();

    int physicsPrecision = 10;
    const float physicsStep = 1.0/60.0f; //delta constant pour la physique

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


        for (int i = 0; i < physicsPrecision; ++i) {
            transformSystem.update(registry); 
            physicsSystem.update(registry, physicsStep / (float) physicsPrecision);
        }

        if (sceneRenderer.isInitialized()) {
            if(!sceneRenderer.render(deltaTime, paused, renderSystem, registry)){
                console.addLog("Scene Renderer error");
            }

        }

        //console.addLog("Physics updates: " + to_string(physicsUpdate));
        //physicsUpdate = 0;

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

    delete testOBBCollider;

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
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && timeSinceKeyPressed >= 1.0f) {
        if (paused)
            paused = false;
        else
            paused = true;
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

    if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && timeSinceKeyPressed >= 0.1f){

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
            rigidBody.colliders.push_back(testOBBCollider);
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
