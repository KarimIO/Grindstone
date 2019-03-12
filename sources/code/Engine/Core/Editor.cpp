#include "Editor.hpp"
#include "../AssetManagers/ImguiManager.hpp"
#include "Engine.hpp"
#include "Scene.hpp"
#include "Space.hpp"
#include "GameObject.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_dock.h"
#include "imgui/imgui_impl_opengl3.h"

Editor::Editor(ImguiManager *manager) {
    show_scene_graph_ = true;
	ImGui::InitDock();
}

void Editor::update() {
    ImGui_ImplOpenGL3_NewFrame();
    //ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

	//ImGui::Begin("Dock Demo", NULL, window_flags);
    //ImGui::BeginDockspace();

    sceneGraphPanel();
    componentPanel();
    assetPanel();
    
    //ImGui::EndDockspace();
	//ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::componentPanel() {
}

void Editor::assetPanel() {
}

void Editor::sceneGraphPanel() {
    if (show_scene_graph_) {
        
        static float f = 0.0f;
        static int counter = 0;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(200, engine.getSettings()->resolution_y_));

        ImGui::Begin("Hello, world!"); 
        // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        /*ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        
        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        
        // Scene Graph:
        for (auto scene : engine.getScenes()) {
            for (auto space : scene->spaces_) {
                for (auto &object : space->objects_) {
                    const char *n = object.getName().c_str();
                    if (ImGui::TreeNode(n))
                        ImGui::Text("%s\n", n);
                }
            }
        }*/
        ImGui::End();
    }
}