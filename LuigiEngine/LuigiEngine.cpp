// Include standard headers
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>

#include "LuigiEngine/SceneMesh.hpp"
#include "Mesh.hpp"

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

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#include "ImGui.hpp"
#include "ImGuiConsole.hpp"
#include "ImGuiHelper.hpp"
#include "SceneRenderer.hpp" // #include "SceneCamera.cpp"

void processInput(GLFWwindow *window);

// settings
constexpr int SCR_WIDTH = 1024;
constexpr int SCR_HEIGHT = 768;

// cameras
/* SceneCamera* cameraWorldSide;
SceneCamera* cameraWorldUp;
SceneCamera* cameraTerrain;
SceneObject* mainCharacter; */

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

    Registry registry;

    TransformSystem transformSystem;
    RenderSystem renderSystem = RenderSystem();
    CameraSystem cameraSystem = CameraSystem();
     // TODO [TP03] Scene Tree

    //SceneObject world;

    // TODO [TP01] Camera - (Model) (View) Projection
    
    Mesh* sphereLOD1 = new Mesh("models/sphereLOD1.obj");
    Mesh* sphereLOD2 = new Mesh("models/sphereLOD2.obj");

    Mesh* suzanneLOD1 = new Mesh("models/suzanneLOD1.obj");
    Mesh* suzanneLOD2 = new Mesh("models/suzanneLOD2.obj");


    GLuint simpleShaders = LoadShaders("shaders/vertex.glsl", "shaders/fragment.glsl");
    GLuint phongShaders = LoadShaders("shaders/vertex_phong.glsl", "shaders/fragment_phong.glsl");
    GLuint terrainShaders = LoadShaders("shaders/vertex_terrain.glsl", "shaders/fragment_terrain.glsl");

    /* quand on cree une mesh attache automatiquement un TextureComponent
    TextureComponent textureComponent;
    textureComponent.texFiles.push_back("sun.jpg");
    textureComponent.texUniforms.push_back("tex"); */

    constexpr double day = 2*M_PI;

    //on pourrait peut creer un ShaderComponent pour savoir quelle shader utilise et stocke les uniforms 
    MeshComponent sphereMeshComponent = MeshComponent({{0, sphereLOD1}, {10, sphereLOD2}}, simpleShaders, {"sun.jpg"}, {"tex"}); 

    MeshComponent earthMeshComponent = MeshComponent({{0, sphereLOD1}, {35, suzanneLOD1}}, simpleShaders, {"earth.jpg"}, {"tex"});

    MeshComponent moonMeshComponent = MeshComponent({{0, sphereLOD1}, {10, sphereLOD2}}, simpleShaders,{"moon.jpg"}, {"tex"} );


    Entity cameraWorldSideEntity = registry.create();

    Entity earthEntity = registry.create();
    Entity moonEntity = registry.create();
    Entity sunEntity = registry.create();

    registry.emplace<MeshComponent>(sunEntity,sphereMeshComponent);
    registry.emplace<MeshComponent>(earthEntity, earthMeshComponent);
    registry.emplace<MeshComponent>(moonEntity, moonMeshComponent);
    
    // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    mat4 pers = perspective(radians(45.0f), 1.0f * SCR_WIDTH / SCR_HEIGHT, 0.1f, 1000.0f);
    
    // on attache les composants aux entite 
    registry.emplace<Transform>(cameraWorldSideEntity).addPos({0, 0, 35});
    registry.emplace<CameraComponent>(cameraWorldSideEntity, pers).speed = 5.0f;

    renderSystem.activeCamera = cameraWorldSideEntity;


    registry.emplace<Transform>(earthEntity).setPos({5, 0, 0});
    registry.emplace<Transform>(moonEntity).setPos({5, 0, 0});
    registry.emplace<Transform>(sunEntity).setPos({0,0,0});

    registry.emplace<Hierarchy>(earthEntity, sunEntity, vector<Entity>{moonEntity});


    registry.get<Transform>(sunEntity).setScale(vec3(2));



    registry.get<Transform>(earthEntity).setScale(vec3(0.5));
    registry.get<Transform>(moonEntity).setScale(vec3(1737.0 / 6378));

 
    Console& console = Console::getInstance();

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
        processInput(window);

        // Debug
        nbLocalMatrixUpdate = new int(0);
        nbGlobalMatrixUpdate = new int(0);
        nbMVPUpdate = new int(0);
        nbViewProjUpdate = new int(0);

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vec3 rotationAngles = vec3(0.0f, currentFrame, 0.0f); 
        quat rotationQuat = quat(rotationAngles);
        registry.get<Transform>(sunEntity).setRot(rotationQuat);

        rotationAngles = vec3(0.0f, currentFrame * 8, 0.0f); 
        rotationQuat = quat(rotationAngles);
        registry.get<Transform>(earthEntity).setRot(rotationQuat);

        rotationAngles = vec3(0.0f, currentFrame * 5, 0.0f); 
        rotationQuat = quat(rotationAngles);
        registry.get<Transform>(moonEntity).setRot(rotationQuat);



        transformSystem.update(registry);
        cameraSystem.update(registry);
        cameraSystem.computeViewProj(registry);


        if (sceneRenderer.isInitialized()) {
            if(!sceneRenderer.render(deltaTime, paused, renderSystem, registry)){
                console.addLog("Scene Renderer error");
            }

        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderImGui();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwSwapInterval(refreshrateMode);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader

    //registry.clear();
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

   /*  // TODO [TP02] Camera - Déplacements
    SceneCamera* mainCamera = SceneObject::getMainCamera();
    if (mainCamera) {
        float distance = mainCamera->speed;

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            distance *= 3;
        if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
            distance /= 3;

        distance *= static_cast<float>(deltaTime);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            mainCamera->transform.addPos(distance * mainCamera->getLocalTarget());
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            mainCamera->transform.addPos(-distance * mainCamera->getLocalTarget());
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            mainCamera->transform.addPos(distance * mainCamera->getLocalRight());
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            mainCamera->transform.addPos(-distance * mainCamera->getLocalRight());
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            mainCamera->transform.addPos(distance * mainCamera->getLocalUp());
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            mainCamera->transform.addPos(-distance * mainCamera->getLocalUp());
        // TODO [Camera] ajouter bind changement de caméra perspective à orthonormal et inversement
        // mat4 orth = ...
        // mainCamera.setProjection(orth);
    } */

    /* if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        SceneObject::setMainCamera(cameraWorldSide);
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        SceneObject::setMainCamera(cameraWorldUp);
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        SceneObject::setMainCamera(cameraTerrain); */

    // TODO [TP04] Déplacement personnage
    /* if (mainCharacter && mainCamera == cameraTerrain) {
        float distance = 0.33f * static_cast<float>(deltaTime);

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            distance *= 3;
        if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
            distance /= 3;

        vec3 basePos = mainCharacter->transform.getPos();
        vec3 movement;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            movement += -distance * vec3(0, 0, 1);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            movement += distance * vec3(0, 0, 1);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            movement += distance * vec3(1, 0, 0);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            movement += -distance * vec3(1, 0, 0);

        // TODO [TP04] Déplacement selon heightmap
        int x = (basePos + movement).x / (terrainSize.x) * heightmapWidth;
        int y = (basePos + movement).z / (terrainSize.y) * heightmapHeight;
        if (y >= 0 && y < heightmapHeight && x >= 0 && x < heightmapWidth) {
            constexpr float offset = 0.1;
            basePos.y = heightmapData[y * heightmapWidth * 3 + x * 3]/255.0f * 1 + offset;
        } else
            basePos.y = 1;
        mainCharacter->transform.setPos(basePos + movement);
    } */

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
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
