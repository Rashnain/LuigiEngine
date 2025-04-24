


#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <vector>
#include <string>

#include "ImGuiHelper.hpp"
#include "ImGuiConsole.hpp"
#include "SceneRenderer.hpp"



void initImGui(GLFWwindow* window) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    // Activer le docking et les viewports
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Optionnel: fenêtres détachables
    
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    style.FrameRounding = 4.0f;
    style.WindowRounding = 0.0f; // Style Unity avec fenêtres carrées
    style.TabRounding = 4.0f;
}



void renderImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Créer un DockSpace sur la fenêtre entière
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    // Important: vérifier le retour de Begin()
    bool dockspace_open = ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);
    
    if (dockspace_open) {
        // Menu principal
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Options")) {
                if (ImGui::MenuItem("Rajouter trucs")) { };
                //if (frustumCulling ? ImGui::MenuItem("Deactivate Fustrum Culling") : ImGui::MenuItem("Activate Fustrum Culling")) { frustumCulling = !frustumCulling ;}
                //if (spacePartitionCulling ? ImGui::MenuItem("Deactivate Space Partition") : ImGui::MenuItem("Activate Space Partition")) { spacePartitionCulling = !spacePartitionCulling ;}
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        // DockSpace
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        
        // La première fois, configurer le layout par défaut (style Unity)
        static bool init_layout = true;
        if (init_layout) {
            init_layout = false;
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            // Créer une disposition en 3 parties: gauche, centre, droite
            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_left_id, dock_center_id, dock_right_id, dock_bottom_id;
            
            // 1. Diviser horizontalement: gauche (hiérarchie) et reste
            ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, &dock_left_id, &dock_main_id);
            
            // 2. Diviser le reste: droite (propriétés) et centre
            ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.4f, &dock_right_id, &dock_center_id);
            
            // 3. Diviser le centre: haut (Vue Scène) et bas (Console)
            ImGui::DockBuilderSplitNode(dock_center_id, ImGuiDir_Down, 0.25f, &dock_bottom_id, &dock_center_id);
            
            // Assigner les fenêtres aux zones
            ImGui::DockBuilderDockWindow("Vue Scène", dock_center_id);
            ImGui::DockBuilderDockWindow("Hiérarchie", dock_left_id);
            ImGui::DockBuilderDockWindow("Propriétés", dock_right_id);
            ImGui::DockBuilderDockWindow("Console", dock_bottom_id);
            
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }
    ImGui::End(); // Fin du DockSpace
    
    // Utiliser le même modèle pour les autres fenêtres
    if (ImGui::Begin("Hiérarchie")) {
    }
    ImGui::End();
    
    if (ImGui::Begin("Propriétés")) {
        // recup id gameobject 
        ImGui::Text("Aucun objet sélectionné");
    }
    ImGui::End();
    
    if (ImGui::Begin("Console")) {
        Console::getInstance().displayLogs();
    }
    ImGui::End();
    
    if (ImGui::Begin("Vue Scène")) {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::SameLine();
        //ImGui::Text("Game Objects Rendered : %d", SceneGraph::getInstance().getNbRenderedGameObjects());

        SceneRenderer & sceneRenderer = SceneRenderer::getInstance();

        if (sceneRenderer.isInitialized()) {
            //if (sceneRenderer.getPauseAnimations() ? ImGui::Button("Animation Paused") : ImGui::Button("Animation Running")){
                //sceneRenderer.setPauseAnimations(!sceneRenderer.getPauseAnimations());
            //}

            // Calculer la taille disponible
            ImVec2 availableSize = ImGui::GetContentRegionAvail();
            
            // Calculer le rapport d'aspect
            float windowAspect = availableSize.x / availableSize.y;
            float textureAspect = (float)sceneRenderer.getframebufferWidth() / sceneRenderer.getframebufferHeight();
            
            // Calculer la taille d'affichage qui maintient l'aspect ratio
            ImVec2 imageSize;
            if (windowAspect > textureAspect) {
                // Fenêtre plus large que l'image
                imageSize = ImVec2(availableSize.y * textureAspect, availableSize.y);
            } else {
                // Fenêtre plus haute que l'image
                imageSize = ImVec2(availableSize.x, availableSize.x / textureAspect);
            }
            
            // Centrer l'image
            float offsetX = (availableSize.x - imageSize.x) * 0.5f;
            float offsetY = (availableSize.y - imageSize.y) * 0.5f;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);
        

            // Afficher l'image
            ImGui::Image((ImTextureID)(intptr_t)sceneRenderer.getTextureColorbuffer(), imageSize, ImVec2(0, 1), ImVec2(1, 0));
            
            // Si vous voulez interagir avec la vue 3D
            if (ImGui::IsItemHovered()) {
                // Traiter les interactions souris avec la vue 3D
            }
        }
        

    }
    ImGui::End();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Pour les fenêtres détachables (si activé)
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

