#include "Engine.hpp"

#ifdef INCLUDE_EDITOR
#include "Editor.hpp"
#include "../AssetManagers/ImguiManager.hpp"
#include "../AssetManagers/TextureManager.hpp"
#include "Scene.hpp"
#include "Space.hpp"
#include "GameObject.hpp"
#include "GraphicsWrapper.hpp"
#include "Texture.hpp"

#include <GL/gl3w.h>
#include "../GraphicsOpenGL/GLRenderTarget.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"

Editor::Editor(ImguiManager *manager) : selected_object_(nullptr) {
	manager_ = manager;
	show_scene_graph_ = true;

	cameras_ = new Camera(engine.getScenes()[0]->spaces_[0], true);
	cameras_->setViewport(200, 200);
	cameras_->initialize();
}

void Editor::update() {
    ImGui_ImplOpenGL3_NewFrame();
    manager_->NewFrame();
    ImGui::NewFrame();

	//ImGui::SetNextWindowPos(ImVec2(0, 40));
	//ImGui::SetNextWindowSize(ImVec2(engine.getSettings()->resolution_x_, engine.getSettings()->resolution_y_ - 40));

	//ImGui::Begin("Dock Demo"); // , NULL, window_flags
    
	prepareDockspace();
    sceneGraphPanel();
    componentPanel();
	assetPanel();
	viewportPanels();
   
	//ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::prepareDockspace() {
	static bool opt_fullscreen_persistant = true;
	static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
	bool opt_fullscreen = opt_fullscreen_persistant;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (opt_flags & ImGuiDockNodeFlags_PassthruDockspace)
		window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", nullptr, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// Dockspace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);
	}
	/*else
	{
		ShowDockingDisabledMessage();
	}*/

	/*if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Docking"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows, 
			// which we can't undo at the moment without finer window depth/z control.
			//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

			if (ImGui::MenuItem("Flag: NoSplit", "", (opt_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 opt_flags ^= ImGuiDockNodeFlags_NoSplit;
			if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (opt_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0))  opt_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;
			if (ImGui::MenuItem("Flag: NoResize", "", (opt_flags & ImGuiDockNodeFlags_NoResize) != 0))                opt_flags ^= ImGuiDockNodeFlags_NoResize;
			if (ImGui::MenuItem("Flag: PassthruDockspace", "", (opt_flags & ImGuiDockNodeFlags_PassthruDockspace) != 0))       opt_flags ^= ImGuiDockNodeFlags_PassthruDockspace;
			if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (opt_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))          opt_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
			ImGui::Separator();
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}*/

	ImGui::End();
}

void Editor::componentPanel() {
	ImGui::Begin("Components");
	if (selected_object_) {
		Scene *scene = engine.getScenes()[0];
		if (scene) {
			Space *space = scene->spaces_[0];
			if (space) {
				for (int i = 0; i < NUM_COMPONENTS - 1; ++i) {
					unsigned int h = selected_object_->getComponentHandle(ComponentType(i));
					if (h != UINT_MAX)
						ImGui::Text("%s %u", component_names[i], h);
				}
				/*static int op = -1;
				if (ImGui::Combo("Add Component", &op, "COMPONENT_TRANSFORM\0COMPONENT_GAME_LOGIC\0COMPONENT_SCRIPT\0COMPONENT_CONTROLLER\0COMPONENT_INPUT\0COMPONENT_RIGID_BODY\0COMPONENT_COLLISION\0COMPONENT_ANIMATION\0COMPONENT_CUBEMAP\0COMPONENT_LIGHT_SPOT\0COMPONENT_LIGHT_POINT\0COMPONENT_LIGHT_DIRECTIONAL\0COMPONENT_CAMERA\0COMPONENT_RENDER_STATIC_MESH\0COMPONENT_RENDER_SKELETAL_MESH\0COMPONENT_RENDER_TERRAIN\0COMPONENT_AUDIO_SOURCE")) {
					//op
				}*/
			}
		}
	}
	else {
		ImGui::Text("Nothing selected.");
	}
	ImGui::End();
}

void Editor::assetPanel() {
	if (show_assets_) {
		ImGui::Begin("Asset Browser");


		
		ImGui::End();
	}
}

void Editor::viewportPanels() {

	cameras_->render();
	engine.getGraphicsWrapper()->BindDefaultFramebuffer(false);

	for (int i = 0; i < 1; ++i) {

		std::string title = "Viewport";
		title += std::to_string(i);
		ImGui::Begin(title.c_str());
		ImVec2 size = ImGui::GetWindowSize();
		cameras_->setViewport(size.x, size.y);

		unsigned int t = ((GLRenderTarget *)cameras_->final_buffer_)->getHandle();
		
		ImGui::GetWindowDrawList()->AddImage(
			(void *)t, ImVec2(ImGui::GetCursorScreenPos()),
			ImVec2(ImGui::GetCursorScreenPos().x + size.x, ImGui::GetCursorScreenPos().y + size.y), ImVec2(0, 1), ImVec2(1, 0));
			
		ImGui::End();
	}
}

void Editor::sceneGraphPanel() {
    if (show_scene_graph_) {
        ImGui::Begin("Scene Graph"); 

        // Scene Graph:
		for (auto scene : engine.getScenes()) {
            for (auto space : scene->spaces_) {
                for (auto &object : space->objects_) {
					std::string n = object.getName();
					if (ImGui::Button(n.c_str())) {
						selected_object_ = &object;
					}
                }
            }
        }
        ImGui::End();
    }
}
#endif