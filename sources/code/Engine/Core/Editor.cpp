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

#include <filesystem>

#include <vector>
#include <algorithm>
#include <iterator>
#include <glm/gtx/transform.hpp>
#include "Renderpaths/RenderPathDeferred.hpp"

Editor::Viewport::Viewport(Camera *c, View v) : camera_(c), pos(1, 1, 1), target(0, 0, 0), first(true) {
	setView(v);
	setViewMatrix();
}

void Editor::Viewport::setViewMatrix() {
	view_mat = glm::lookAt(pos, target, up);
}

void Editor::Viewport::setView(View v) {
	view = v;
	target = glm::vec3(0, 0, 0);
	up = glm::vec3(0, 1, 0);
	
	if (v == Perspective) {
		camera_->setPerspective();
	}
	else {
		camera_->setOrtho(-10, 10, 10, -10);
	}

	double d = 30;

	switch (v) {
	case Viewport::View::Top:
		pos = glm::vec3(0, d, 0);
		up = glm::vec3(1, 0, 0);
		break;
	case Viewport::View::Bottom:
		pos = glm::vec3(0, -d, 0);
		up = glm::vec3(-1, 0, 0);
		break;
	case Viewport::View::Left:
		pos = glm::vec3(d, 0, 0);
		break;
	case Viewport::View::Right:
		pos = glm::vec3(-d, 0, 0);
		break;
	case Viewport::View::Front:
		pos = glm::vec3(0, 0, d);
		break;
	case Viewport::View::Back:
		pos = glm::vec3(0, 0, -d);
		break;
	default:
	case Viewport::View::Perspective:
		pos = glm::vec3(1, 1, 1);
		break;
	}
}

Editor::Editor(ImguiManager *manager) : selected_object_(nullptr) {
	manager_ = manager;
	show_scene_graph_ = true;
	show_asset_browser_ = true;
	show_viewport_= true;
	show_component_panel_ = true;
	
	Camera *c = new Camera(engine.getScenes()[0]->spaces_[0], true);
	c->setViewport(800, 600);
	c->initialize();
	((RenderPathDeferred *)c->render_path_)->wireframe_ = true;
	viewports_.emplace_back(c, Viewport::View::Top);
	c = new Camera(engine.getScenes()[0]->spaces_[0], true);
	c->setViewport(800, 600);
	c->initialize();
	viewports_.emplace_back(c, Viewport::View::Perspective);

	asset_path_ = "../assets";
	next_asset_path_ = "";
	getDirectory();
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

	if (next_asset_path_ != "" && next_asset_path_ != asset_path_) {
		asset_path_ = next_asset_path_;
		next_asset_path_ = "";
		getDirectory();
	}
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

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Close", "", false))
				engine.shutdown();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Show Asset Browser", "", show_asset_browser_)) show_asset_browser_ = !show_asset_browser_;
			if (ImGui::MenuItem("Show Scene Graph", "", show_scene_graph_)) show_scene_graph_ = !show_scene_graph_;
			if (ImGui::MenuItem("Show Component Panel", "", show_component_panel_)) show_component_panel_ = !show_component_panel_;
			if (ImGui::MenuItem("Show Viewport Panel", "", show_viewport_)) show_viewport_ = !show_viewport_;
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	ImGui::End();
}

void Editor::componentPanel() {
	if (show_component_panel_) {
		ImGui::Begin("Components Panel", &show_component_panel_);
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
}

void Editor::getDirectory() {
	directories_.clear();
	files_.clear();

	if (asset_path_ != "../assets") {
		size_t d1 = asset_path_.find_last_of('\\');
		size_t d2 = asset_path_.find_last_of('/');
		size_t d = (d1 > d2) ? d1 : d2;
		std::string p = asset_path_.substr(0, d);
		directories_.emplace_back(p, "^ Up ^");
	}

	for (auto & p : std::filesystem::directory_iterator(asset_path_)) {
		std::string path = p.path().string();
		std::string name = p.path().filename().string();
		if (p.is_directory()) {
			directories_.emplace_back(path, name);
		}
		else {
			files_.emplace_back(path, name);
		}
	}
}

void Editor::assetPanel() {
	if (show_asset_browser_) {
		ImGui::Begin("Asset Browser", &show_asset_browser_);

		ImVec2 button_sz(80, 30);
		ImGuiStyle& style = ImGui::GetStyle();
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
		for (int n = 0; n < directories_.size(); n++) {
			ImGui::PushID(n);
			if (ImGui::Button(directories_[n].name.c_str(), button_sz)) {
				next_asset_path_ = directories_[n].path;
			}
			float last_button_x2 = ImGui::GetItemRectMax().x;
			float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
			if (n + 1 < directories_.size() && next_button_x2 < window_visible_x2)
				ImGui::SameLine();
			ImGui::PopID();
		}

		ImGui::Separator();

		for (int n = 0; n < files_.size(); n++) {
			ImGui::PushID(n);
			ImGui::Button(files_[n].name.c_str(), button_sz);
			float last_button_x2 = ImGui::GetItemRectMax().x;
			float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
			if (n + 1 < files_.size() && next_button_x2 < window_visible_x2)
				ImGui::SameLine();
			ImGui::PopID();
		}

		ImGui::End();
	}
}

void Editor::viewportPanels() {
	if (show_viewport_) {
		int i = 0;
		for (auto &v : viewports_) {

			std::string title = "Viewport ";
			title += std::to_string(++i);

			ImGui::Begin(title.c_str(), &show_viewport_);
			ImVec2 size = ImGui::GetWindowSize();
			ImGuiStyle& style = ImGui::GetStyle();
			size.x -= style.FramePadding.x * 2;
			size.y -= style.FramePadding.y * 2;
			/*if (v.first) {
				v.first = false;
				v.camera_->setViewport(size.x, size.y);
				//v.camera_->initialize();
			}*/

			v.camera_->render(v.pos, v.view_mat);
			engine.getGraphicsWrapper()->BindDefaultFramebuffer(false);

			unsigned int t = ((GLRenderTarget *)v.camera_->final_buffer_)->getHandle();

			ImGui::GetWindowDrawList()->AddImage(
				(void *)t, ImVec2(ImGui::GetCursorScreenPos()),
				ImVec2(ImGui::GetCursorScreenPos().x + size.x, ImGui::GetCursorScreenPos().y + size.y), ImVec2(0, 1), ImVec2(1, 0));

			static bool pp;

			const char* items[] = { "Up", "Down", "Left", "Right", "Forward", "Back", "Perspective" };
			if (v.view_option == nullptr) {
				v.view_option = items[v.view];
			}

			ImGui::PushItemWidth(100);
			if (ImGui::BeginCombo("##combo", v.view_option)) {
				for (unsigned int n = 0; n < IM_ARRAYSIZE(items); n++) {
					bool is_selected = (v.view_option == items[n]); // You can store your selection however you want, outside or inside your objects
					if (ImGui::Selectable(items[n], is_selected)) {
						v.view_option = items[n];
						v.setView((Viewport::View)n);
						v.setViewMatrix();
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
				}
				ImGui::EndCombo();
			}

			const char* debug_combo_items[] = { "Lit", "Distance", "Normal", "View Normal", "Albedo", "Specular", "Roughness", "Position" };
			if (v.debug_combo_option == nullptr)
				v.debug_combo_option = debug_combo_items[0];

			if (ImGui::TreeNode("View Data")) {
				ImGui::Checkbox("Wireframe", &((RenderPathDeferred *)v.camera_->render_path_)->wireframe_);

				ImGui::Separator();

				if (ImGui::TreeNode("Post-Processing")) {
					ImGui::Checkbox("SSAO", &pp);
					ImGui::Checkbox("Reflections", &pp);
					ImGui::Checkbox("Auto-Exposure", &pp);
					ImGui::Checkbox("Color Grading", &pp);

					ImGui::TreePop();
				}

				ImGui::Separator();

				ImGui::PushItemWidth(100);
				if (ImGui::BeginCombo("##debug_combo", v.debug_combo_option)) {
					for (int n = 0; n < IM_ARRAYSIZE(debug_combo_items); n++) {
						bool is_selected = (v.debug_combo_option == debug_combo_items[n]); // You can store your selection however you want, outside or inside your objects
						if (ImGui::Selectable(debug_combo_items[n], is_selected)) {
							v.debug_combo_option = debug_combo_items[n];
							v.camera_->render_path_->setDebugMode(n);
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
					}
					ImGui::EndCombo();
				}

				ImGui::TreePop();
			}

			ImGui::End();
		}
	}
}

void Editor::sceneGraphPanel() {
	if (show_scene_graph_) {
		ImGui::Begin("Scene Graph", &show_scene_graph_);

		if (ImGui::Button("Add GameObject")) {
			engine.getScenes()[0]->spaces_[0]->objects_.emplace_back(engine.getScenes()[0]->spaces_[0]->objects_.size(), "New Object");
		}
		ImGui::Separator();

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