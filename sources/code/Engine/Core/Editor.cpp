#include "Engine.hpp"

#ifdef INCLUDE_EDITOR
#include "Space.hpp"
#include "Editor.hpp"
#include "Core/Input.hpp"
#include "../AssetManagers/ImguiManager.hpp"
#include "../AssetManagers/TextureManager.hpp"
#include "Scene.hpp"
#include "Space.hpp"
#include "GameObject.hpp"
#include "GraphicsWrapper.hpp"
#include "Texture.hpp"
#include "../Systems/CubemapSystem.hpp"

#include <GL/gl3w.h>
#include "../GraphicsOpenGL/GLRenderTarget.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"

#include <rapidjson/prettywriter.h>

#include <filesystem>
#include <fstream>

#include <vector>
#include <algorithm>
#include <iterator>
#include <glm/gtx/transform.hpp>
#include "Renderpaths/RenderPathDeferred.hpp"
#include "Systems/TransformSystem.hpp"
#include "Systems/LightSpotSystem.hpp"
#include "Utilities/Logger.hpp"

#include "../Converter/ImageConverter.hpp"
#include "../Converter/Utilities.hpp"

Editor::Viewport::Viewport(Camera *c, View v) : camera_(c), first(true) {
	setView(v);
}

void Editor::setPath(std::string path) {
	path_ = path;
}

void Editor::Viewport::calcDirs() {
	fwd = glm::vec3(
		glm::cos(angles.x) * glm::sin(angles.y),
		glm::sin(angles.x),
		glm::cos(angles.x) * glm::cos(angles.y)
	);

	right = glm::vec3(
		glm::sin(angles.y - 3.14159f / 2.0f),
		0,
		glm::cos(angles.y - 3.14159f / 2.0f)
	);
	
	up = glm::cross(right, fwd);
}

void Editor::Viewport::setViewMatrix() {
	view_mat = glm::lookAt(pos, pos + fwd, up);
}

void Editor::Viewport::setView(View v) {
	view = v;

	if (v == Perspective) {
		camera_->setPerspective();
	}
	else {
		camera_->setOrtho(-10, 10, 10, -10);
	}

	double d = 30;
	float qpi = glm::pi<float>() / 2.0f;

	switch (v) {
	case Viewport::View::Top:
		pos = glm::vec3(0, d, 0);
		angles = glm::vec3(-qpi, 0.0, 0.0);
		break;
	case Viewport::View::Bottom:
		pos = glm::vec3(0, -d, 0);
		angles = glm::vec3(qpi, 0.0, 0.0);
		break;
	case Viewport::View::Left:
		pos = glm::vec3(d, 0, 0);
		angles = glm::vec3(0.0, -qpi, 0.0);
		break;
	case Viewport::View::Right:
		pos = glm::vec3(-d, 0, 0);
		angles = glm::vec3(0.0, qpi, 0.0);
		break;
	case Viewport::View::Front:
		pos = glm::vec3(0, 0, d);
		angles = glm::vec3(0.0, qpi * 2.0f, 0.0);
		break;
	case Viewport::View::Back:
		pos = glm::vec3(0, 0, -d);
		angles = glm::vec3(0.0, 0.0, 0.0);
		break;
	default:
	case Viewport::View::Perspective:
		pos = glm::vec3(-1, 2, -1);
		angles = glm::vec3(-0.78, 0.78, 0.78);
		break;
	}

	calcDirs();
	setViewMatrix();
}

Editor::Editor(ImguiManager *manager) : selected_object_handle_(-1) {
	manager_ = manager;
	show_scene_graph_ = true;
	show_asset_browser_ = true;
	show_viewport_= true;
	show_component_panel_ = true;

	obj_name = new char[128];

	viewport_manipulating_ = 0;
	
	Camera *c = new Camera(engine.getScenes()[0]->spaces_[0], true);
	c->setViewport(800, 600);
	c->enable_reflections_ = false;
	c->enable_auto_exposure_ = false;
	c->initialize();
	((RenderPathDeferred *)c->render_path_)->wireframe_ = true;
	viewports_.emplace_back(c, Viewport::View::Top);
	c = new Camera(engine.getScenes()[0]->spaces_[0], true);
	c->enable_reflections_ = true;
	c->enable_auto_exposure_ = true;
	c->setViewport(800, 600);
	c->initialize();
	viewports_.emplace_back(c, Viewport::View::Perspective);

	asset_path_ = "../assets";
	next_asset_path_ = "";
	getDirectory();
	next_refresh_asset_directory_time_ = 0;
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
			if (ImGui::MenuItem("Save", "", false))
				saveFile();
			if (ImGui::MenuItem("Save As", "", false))
				saveFileAs();
			ImGui::Separator();
			if (ImGui::MenuItem("Load", "", false))
				loadFile();
			if (ImGui::MenuItem("Load From", "", false))
				loadFileFrom();
			ImGui::Separator();
			if (ImGui::MenuItem("Import", "", false))
				importFile();
			ImGui::Separator();
			if (ImGui::MenuItem("Close", "", false))
				engine.shutdown();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Show Asset Browser", "", show_asset_browser_)) show_asset_browser_ = !show_asset_browser_;
			if (ImGui::MenuItem("Show Scene Graph", "", show_scene_graph_)) show_scene_graph_ = !show_scene_graph_;
			if (ImGui::MenuItem("Show Component Panel", "", show_component_panel_)) show_component_panel_ = !show_component_panel_;
			if (ImGui::MenuItem("Add Viewport Panel", "", false)) {
				Camera *c = new Camera(engine.getScenes()[0]->spaces_[0], true);
				c->enable_reflections_ = true;
				c->enable_auto_exposure_ = true;
				c->setViewport(800, 600);
				c->initialize();
				viewports_.emplace_back(c, Viewport::View::Perspective);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Build")) {
			auto space = engine.getScene(0)->spaces_[0];
			auto cubemap_sys = (CubemapSubSystem *)space->getSubsystem(COMPONENT_CUBEMAP);

			//if (ImGui::MenuItem("Build Irradiance Probes", "", false)) {
			if (ImGui::MenuItem("Build Light Probes", "", false)) {
				cubemap_sys->bake();
			}
			
			/*if (ImGui::MenuItem("Build Reflection Probes", "", false)) {
				
			}
			
			if (ImGui::MenuItem("Build Lightmap", "", false)) {
				
			}
			
			if (ImGui::MenuItem("Build Lighting", "", false)) {
				cubemap_sys->bake();	// Build Irradiance
				// Build Reflection Probes
				// Build Lightmap
			}*/

			ImGui::EndMenu();
		}

		if (!engine.edit_is_simulating_) {
			if (ImGui::BeginMenu("Simulate")) {
				engine.startSimulation();
				ImGui::EndMenu();
			}
		}
		else {
			if (ImGui::BeginMenu("Stop Simulation")) {
				engine.stopSimulation();
				ImGui::EndMenu();
			}
		}

		ImGui::EndMenuBar();
	}

	ImGui::End();
}

void Editor::componentPanel() {
	if (show_component_panel_) {
		ImGui::Begin("Components Panel", &show_component_panel_);
		if (selected_object_handle_ != -1) {
			Scene *scene = engine.getScenes()[0];
			if (scene) {
				Space *space = scene->spaces_[0];
				if (space) {
					GameObject *selected_object_ = &space->getObject(selected_object_handle_);
					if (ImGui::InputText("Object Name", obj_name, 128)) {
						selected_object_->setName(obj_name);
					}

					ImGui::Separator();

					int op = 0;
					if (ImGui::BeginCombo("##addcompcombo", "Add a component")) {
						for (unsigned int n = 0; n < IM_ARRAYSIZE(component_names); n++) {
							bool is_selected = (op == n); // You can store your selection however you want, outside or inside your objects
							if (ImGui::Selectable(component_names[n], is_selected)) {
								ComponentType ct = (ComponentType)n;
								ComponentHandle comp = space->getSubsystem(ct)->addComponent(selected_object_->getID());
								selected_object_->setComponentHandle(ct, comp);
							}
						}
						ImGui::EndCombo();
					}

					for (int i = 0; i < NUM_COMPONENTS - 1; ++i) {
						unsigned int h = selected_object_->getComponentHandle(ComponentType(i));
						if (h != UINT_MAX) { // Don't show unused components
							displayComponent(ComponentType(i), h);
						}
					}
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

void vec3Component(std::string base, glm::vec3 &val, float w) {
	float x = val.x;
	float y = val.y;
	float z = val.z;
	ImGui::PushItemWidth(w);
	ImGui::PushItemWidth(w);
	ImGui::PushItemWidth(w);
	if (ImGui::InputFloat((base + "X").c_str(), &x))
		val.x = x;
	ImGui::SameLine();
	if (ImGui::InputFloat((base + "Y").c_str(), &y))
		val.y = y;
	ImGui::SameLine();
	if (ImGui::InputFloat((base + "Z").c_str(), &z))
		val.z = z;
}

void floatComponent(std::string text, std::string base, float &val) {
	float x = val;
	ImGui::Text(text.c_str());
	if (ImGui::InputFloat(base.c_str(), &x))
		val = x;
}

void vec4Component(std::string base, glm::vec3 &val, float &vala, float w) {
	float x = val.x;
	float y = val.y;
	float z = val.z;
	float a = vala;
	ImGui::PushItemWidth(w);
	ImGui::PushItemWidth(w);
	ImGui::PushItemWidth(w);
	ImGui::PushItemWidth(w);
	if (ImGui::InputFloat((base + "X").c_str(), &x))
		val.x = x;
	ImGui::SameLine();
	if (ImGui::InputFloat((base + "Y").c_str(), &y))
		val.y = y;
	ImGui::SameLine();
	if (ImGui::InputFloat((base + "Z").c_str(), &z))
		val.z = z;
	ImGui::SameLine();
	if (ImGui::InputFloat((base + "A").c_str(), &a))
		vala = a;
}

void Editor::displayComponent(ComponentType type, ComponentHandle handle) {
	if (ImGui::TreeNode(component_names[type])) {
		ImGuiStyle& style = ImGui::GetStyle();
		float w = ImGui::GetWindowContentRegionMax().x;
		float p = style.FramePadding.x;
		float w3 = (w / 3.0f) - p;
		w3 = (w3 > 30.0f) ? w3 : 30.0f;
		float w4 = (w / 3.0f) - p;
		w4 = (w4 > 30.0f) ? w4 : 30.0f;

		// Component Settings
		switch (type) {
		case COMPONENT_TRANSFORM: {
			TransformComponent &tr = ((TransformSubSystem *)engine.getScenes()[0]->spaces_[0]->getSubsystem(COMPONENT_TRANSFORM))->getComponent(handle);
			ImGui::Text("Position"); 
			vec3Component("##t", tr.position_, w3);

			float rx = glm::degrees(tr.angles_.x);
			float ry = glm::degrees(tr.angles_.y);
			float rz = glm::degrees(tr.angles_.z);
			ImGui::Text("Rotation");
			ImGui::PushItemWidth(w4);
			ImGui::PushItemWidth(w4);
			ImGui::PushItemWidth(w4);
			if (ImGui::InputFloat("##rX", &rx))
				tr.angles_.x = glm::radians(rx);
			ImGui::SameLine();
			if (ImGui::InputFloat("##rY", &ry))
				tr.angles_.y = glm::radians(ry);
			ImGui::SameLine();
			if (ImGui::InputFloat("##rZ", &rz))
				tr.angles_.z = glm::radians(rz);

			float sx = tr.scale_.x;
			float sy = tr.scale_.y;
			float sz = tr.scale_.z;
			ImGui::Text("Scale");
			ImGui::PushItemWidth(w3);
			ImGui::PushItemWidth(w3);
			ImGui::PushItemWidth(w3);
			if (ImGui::InputFloat("##sX", &sx) && sx != 0.0f)
				tr.scale_.x = sx;
			ImGui::SameLine();
			if (ImGui::InputFloat("##sY", &sy) && sy != 0.0f)
				tr.scale_.y = sy;
			ImGui::SameLine();
			if (ImGui::InputFloat("##sZ", &sz) && sz != 0.0f)
				tr.scale_.z = sz;
			break;
		}
		case COMPONENT_LIGHT_SPOT: {
			LightSpotComponent &tr = ((LightSpotSubSystem *)engine.getScenes()[0]->spaces_[0]->getSubsystem(COMPONENT_LIGHT_SPOT))->getComponent(handle);
			ImGui::Text("Color");
			vec4Component("##lsc", tr.properties_.color, tr.properties_.power, w4);

			floatComponent("Inner Angle", "##lsi", tr.properties_.innerAngle);
			floatComponent("Outer Angle", "##lso", tr.properties_.outerAngle);
			floatComponent("Attuenuation Radius", "##lsa", tr.properties_.attenuationRadius);
			ImGui::Checkbox("Shadow Enabled", &tr.properties_.shadow);
			break;
		}
		default: break;
		}

		ImGui::TreePop();
	}
}

void Editor::assetPanel() {
	if (show_asset_browser_) {
		double time = engine.getTimeCurrent();
		if (next_refresh_asset_directory_time_ <= time) {
			getDirectory();
			next_refresh_asset_directory_time_ = time + 10.0;
		}

		ImGui::Begin("Asset Browser", &show_asset_browser_);
		if (ImGui::BeginPopupContextItem("Asset Browser Rt")) {
			ImGui::BeginMenu("Create Folder");
			ImGui::BeginMenu("Create File");
			ImGui::EndPopup();
		}

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

#include "Systems/CameraSystem.hpp"

void Editor::viewportPanels() {
	if (show_viewport_) {
		if (engine.edit_is_simulating_) {
			ImGui::Begin("Simulation Viewport", &show_viewport_);
			ImVec2 size = ImGui::GetWindowSize();

			Space *space = engine.getScene(0)->spaces_[0];
			CameraSubSystem *camsys = (CameraSubSystem *)space->getSubsystem(COMPONENT_CAMERA);
			CameraComponent &camcomp = camsys->getComponent(0);
			GameObjectHandle obj = camcomp.game_object_handle_;
			ComponentHandle transcomp = space->getObject(obj).getComponentHandle(COMPONENT_TRANSFORM);
			TransformSubSystem *transys = (TransformSubSystem *)space->getSubsystem(COMPONENT_TRANSFORM);
			
			glm::vec3 pos = transys->getPosition(transcomp);
			glm::vec3 fwd = transys->getForward(transcomp);
			glm::vec3 up = transys->getUp(transcomp);

			glm::mat4 view = glm::lookAt(
				pos,
				pos + fwd,
				up
			);

			Camera &cam = camcomp.camera_;

			cam.setViewport(size.x, size.y);
			cam.render(pos, view);
			engine.getGraphicsWrapper()->BindDefaultFramebuffer(false);

			unsigned int t = ((GLRenderTarget *)cam.final_buffer_)->getHandle();

			ImGui::GetWindowDrawList()->AddImage(
				(void *)t, ImVec2(ImGui::GetCursorScreenPos()),
				ImVec2(ImGui::GetCursorScreenPos().x + size.x, ImGui::GetCursorScreenPos().y + size.y), ImVec2(0, 1), ImVec2(1, 0));

			ImGui::End();
		}
		else {
			bool viewport_selected = false;
			bool right_clicking = engine.getInputManager()->GetMouseButton(MOUSE_RIGHT) > 0;

			auto mouse = ImGui::GetMousePos();

			double dt = engine.getUpdateTimeDelta();

			int i = 0;
			for (auto &v : viewports_) {
				std::string title = "Viewport ";
				title += std::to_string(++i);

				ImGui::Begin(title.c_str(), &show_viewport_);
				ImVec2 size = ImGui::GetWindowSize();
				/*ImGuiStyle& style = ImGui::GetStyle();
				size.x -= style.FramePadding.x * 2;
				size.y -= style.FramePadding.y * 2;*/

				/*if (v.first) {
					v.first = false;
					v.camera_->setViewport(size.x, size.y);
					//v.camera_->initialize();
				}*/

				//if (viewport_manipulating_) {
				if (right_clicking) {
					auto min = ImGui::GetWindowPos();
					auto max = min;
					max.x += size.x;
					max.y += size.y;

					if (viewport_manipulating_ == i) {
						float msensitivity = 0.5f;
						float keymovespeed = 8.0f;
						float cursormovespeed = 2.0f;

						int midx = int(min.x + max.x) / 2;
						int midy = int(min.y + max.y) / 2;

						float cx = (midx - mouse.x);
						float cy = (midy - mouse.y);

						float ox = (((engine.getInputManager()->GetKey(KEY_W) > 0) ? keymovespeed : 0) - ((engine.getInputManager()->GetKey(KEY_S) > 0) ? keymovespeed : 0));
						float oy = (((engine.getInputManager()->GetKey(KEY_D) > 0) ? keymovespeed : 0) - ((engine.getInputManager()->GetKey(KEY_A) > 0) ? keymovespeed : 0));

						if (v.view == Viewport::View::Perspective) {
							v.angles.x += float(dt) * msensitivity * cy;
							v.angles.y += float(dt) * msensitivity * cx;

							float hpi = glm::pi<float>() / 2.0f;

							v.angles.x = glm::clamp(v.angles.x, -hpi, hpi);

							float oz = 8.0f * ((engine.getInputManager()->GetKey(KEY_SPACE) > 0) ? 1 : 0) - ((engine.getInputManager()->GetKey(KEY_CONTROL) > 0) ? 1 : 0);

							v.pos += (float)dt * (ox * v.fwd + oy * v.right + oz * v.up);
						}
						else {
							cx *= cursormovespeed;
							cy *= cursormovespeed;
							v.pos += (float)dt * ((cx + oy) * v.right + (-cy + ox) * v.up);
						}


						v.calcDirs();
						v.setViewMatrix();

						engine.getGraphicsWrapper()->SetCursor(midx, midy);
						viewport_selected = true;
					}
					else if (viewport_manipulating_ == 0) {
						if (mouse.x < max.x && mouse.x > min.x && mouse.y > min.y && mouse.y < max.y) {
							viewport_manipulating_ = i;

							int midx = int(min.x + max.x) / 2;
							int midy = int(min.y + max.y) / 2;

							engine.getGraphicsWrapper()->SetCursor(midx, midy);
						}
					}
				}
				else {
					viewport_manipulating_ = 0;
				}

				// v.camera_->setViewport(size.x, size.y);
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
						ImGui::Checkbox("Auto Exposure", &v.camera_->enable_auto_exposure_);
						ImGui::Checkbox("Reflections", &v.camera_->enable_reflections_);
						//ImGui::Checkbox("Auto-Exposure", &pp);
						//ImGui::Checkbox("Color Grading", &pp);

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
}

void Editor::saveFile() {
	saveFile(path_, engine.getScenes()[0]);
}

void Editor::saveFileAs() {
	std::string p = engine.getGraphicsWrapper()->getSavePath("JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0", "json");
	//loadFile(path_, engine.getScenes()[0]);
	if (p != "") {
		path_ = p;
		saveFile(path_, engine.getScenes()[0]);
	}
}

void Editor::saveFile(std::string path, Scene *scene) {

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	writer.StartObject();
	writer.Key("name");
	writer.String("New Scene");

	writer.Key("spaces");
	writer.StartObject();

	for (auto space : scene->spaces_) {
		writer.Key(space->getName().c_str());
		writer.StartObject();

		for (auto &object : space->objects_) {
			writer.Key(object.getName().c_str());
			writer.StartObject();

			for (int i = 0; i < NUM_COMPONENTS; ++i) {
				ComponentHandle h = object.getComponentHandle((ComponentType)i);
				if (h != -1) {
					writer.Key(component_names[i]);
					writer.StartObject();
					space->getSubsystem((ComponentType)i)->writeComponentToJson(h, writer);
					writer.EndObject();
				}
			}

			writer.EndObject();
		}

		writer.EndObject();
	}

	writer.EndObject();
	writer.EndObject();

	std::ofstream ofile(path);
	if (ofile.is_open()) {
		GRIND_LOG("Writing to: {0}.\n", path);
		ofile << buffer.GetString();
	}
	else {
		GRIND_WARN("Could not write to file {0}.\n", path);
	}

	ofile.close();
}

void Editor::cleanCameras() {
	for (auto v : viewports_) {
		delete v.camera_;
	}

	viewports_.clear();

	Camera *c = new Camera(engine.getScenes()[0]->spaces_[0], true);
	c->setViewport(800, 600);
	c->enable_reflections_ = false;
	c->enable_auto_exposure_ = false;
	c->initialize();
	((RenderPathDeferred *)c->render_path_)->wireframe_ = true;
	viewports_.emplace_back(c, Viewport::View::Top);
	c = new Camera(engine.getScenes()[0]->spaces_[0], true);
	c->enable_reflections_ = true;
	c->enable_auto_exposure_ = true;
	c->setViewport(800, 600);
	c->initialize();
	viewports_.emplace_back(c, Viewport::View::Perspective);

}

void Editor::importFile() {
	std::string path = engine.getGraphicsWrapper()->getLoadPath("Image File\0*.png;*.tga;*.jpg;*.jpeg;*.bmp;*.psd;*.gif;*.hdr;*.pic\0Model Files\0*.3ds;*.dxf;*.fbx;*.obj;*.blend;*.dae\0All Files (*.*)\0*.*\0", "png");

	GRIND_LOG("Importing {0}", path);

	size_t posslash = path.find_last_of('\\');
	posslash = (posslash == -1) ? 0 : posslash;
	size_t posslashf = path.find_last_of('/');
	posslashf = (posslashf == -1) ? 0 : posslashf;
	if (posslashf > posslash) posslash = posslashf;

	size_t posdot = path.find_last_of('.');
	std::string base = path.substr(posslash + 1, posdot - posslash - 1);
	std::string ext = path.substr(posdot + 1);

	if (ext == "png" || ext == "tga" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "psd" || ext == "gif" || ext == "hdr" || ext == "pic") {
		std::string final = asset_path_ + "/" + base + ".dds";
		GRIND_WARN("Importing Image {0}", final);

		ConvertTexture(path, false, final, Compression::C_DETECT);
	}
	else if (ext == "3ds" || ext == "dxf" || ext == "fbx" || ext == "obj" || ext == "blend" || ext == "dae") {
		GRIND_WARN("Importing Model");
	}
	else {
		GRIND_WARN("Invalid file type: {0}", ext);
	}

	getDirectory();
	next_refresh_asset_directory_time_ = engine.getTimeCurrent() + 10.0;
}

void Editor::loadFileFrom() {
	std::string p = engine.getGraphicsWrapper()->getLoadPath("JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0", "json");
	//loadFile(path_, engine.getScenes()[0]);
	if (p != "") {
		path_ = p;
		Scene *s = engine.getScenes()[0];
		if (s) {
			delete s;
		}
		engine.getScenes().clear();
		engine.addScene(path_);
		cleanCameras();
	}
}

void Editor::loadFile() {
	engine.getScenes()[0]->reload();
	cleanCameras();
}

void Editor::sceneGraphPanel() {
	if (show_scene_graph_) {
		ImGui::Begin("Scene Graph", &show_scene_graph_);

		if (ImGui::Button("Add GameObject")) {
			engine.getScenes()[0]->spaces_[0]->objects_.emplace_back((GameObjectHandle)engine.getScenes()[0]->spaces_[0]->objects_.size(), "New Object");
		}
		ImGui::Separator();

        // Scene Graph:
		for (auto scene : engine.getScenes()) {
            for (auto space : scene->spaces_) {
                for (auto &object : space->objects_) {
					std::string n = object.getName();
					if (ImGui::Button(n.c_str())) {
						selected_object_handle_ = object.getID();
						auto s = object.getName().size();
						memset(obj_name + s, 0, 256 - s);
						memcpy(obj_name, object.getName().c_str(), s);
					}
                }
            }
        }
        ImGui::End();
    }
}
#endif