#include "Engine.hpp"

#include "GraphicsWrapper.hpp"

#undef Bool

#ifdef INCLUDE_EDITOR
#include "Space.hpp"
#include "Editor.hpp"
#include "Core/Input.hpp"
#include "../AssetManagers/ImguiManager.hpp"
#include "../AssetManagers/TextureManager.hpp"
#include "Scene.hpp"
#include "Space.hpp"
#include "GameObject.hpp"
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
#include "Systems/RigidBodySystem.hpp"

#include "../Converter/ImageConverter.hpp"
#include "../Converter/Utilities.hpp"

Editor::Viewport::Viewport(Camera *c, View v) : camera_(c), first(true), viewport_shown(true) {
	setView(v);
}

void Editor::refreshSceneGraph() {
	/*for (auto &o : engine.getScene(0)->spaces_[0]->objects_) {
		GameObjectHandle h = o.getParentID();
		if (h == -1) {
			scene_graph_.push_back(new SceneGraphNode(h, {}));
		}
		else {
			std::vector<SceneGraphNode *> &s = scene_graph_;
			while (s.size() > 0 && s.back()->object_handle_ != h) {
				s = s.back()->children_;
			}
			s.push_back(new SceneGraphNode(h, {}));
		}
	}*/
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
// setViewMatrix();
}

Editor::Editor(ImguiManager *manager) : selected_object_handle_(-1) {
	manager_ = manager;
	show_scene_graph_ = true;
	show_asset_browser_ = true;
	show_viewport_ = true;
	show_console_ = true;
	show_inspector_panel_ = true;

	obj_name = new char[128];
	console_buffer_[0] = 0;
	console_entries_first_ = 0;
	console_entries_count_ = 0;

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
	refreshSceneGraph();
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
	inspectorPanel();
	assetPanel();
	consolePanel();
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

void Editor::reload(ImguiManager * manager) {
	manager_ = manager;

	for (auto &v : viewports_) {
		v.camera_->reloadGraphics();
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
			if (ImGui::MenuItem("Show Component Panel", "", show_inspector_panel_)) show_inspector_panel_ = !show_inspector_panel_;
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
				engine.getSystem<CubemapSystem>()->bake();
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

bool vec2Component(std::string base, glm::vec2 &val, float w) {
	float x = val.x;
	float y = val.y;
	if (w > 0.0f) {
		ImGui::PushItemWidth(w);
		ImGui::PushItemWidth(w);
	}

	bool changed = false;
	if (ImGui::InputFloat((base + "X").c_str(), &x)) {
		val.x = x;
		changed = true;
	}

	ImGui::SameLine();
	if (ImGui::InputFloat((base + "Y").c_str(), &y)) {
		val.y = y;
		changed = true;
	}

	return changed;
}

bool vec3Component(std::string base, glm::vec3 &val, float w) {
	float x = val.x;
	float y = val.y;
	float z = val.z;
	if (w > 0.0f) {
		ImGui::PushItemWidth(w);
		ImGui::PushItemWidth(w);
		ImGui::PushItemWidth(w);
	}

	bool changed = false;
	if (ImGui::InputFloat((base + "X").c_str(), &x)) {
		val.x = x;
		changed = true;
	}

	ImGui::SameLine();
	if (ImGui::InputFloat((base + "Y").c_str(), &y)) {
		val.y = y;
		changed = true;
	}

	ImGui::SameLine();
	if (ImGui::InputFloat((base + "Z").c_str(), &z)) {
		val.z = z;
		changed = true;
	}

	return changed;
}

bool vec4Component(std::string base, glm::vec4 &val, float w) {
	float x = val.x;
	float y = val.y;
	float z = val.z;
	float a = val.z;
	if (w > 0.0f) {
		ImGui::PushItemWidth(w);
		ImGui::PushItemWidth(w);
		ImGui::PushItemWidth(w);
		ImGui::PushItemWidth(w);
	}

	bool changed = false;
	if (ImGui::InputFloat((base + "X").c_str(), &x)) {
		val.x = x;
		changed = true;
	}

	ImGui::SameLine();
	if (ImGui::InputFloat((base + "Y").c_str(), &y)) {
		val.y = y;
		changed = true;
	}

	ImGui::SameLine();
	if (ImGui::InputFloat((base + "Z").c_str(), &z)) {
		val.z = z;
		changed = true;
	}

	ImGui::SameLine();
	if (ImGui::InputFloat((base + "W").c_str(), &z)) {
		val.z = z;
		changed = true;
	}

	return changed;
}

bool floatComponent(float w, std::string base, float &val) {
	ImGui::PushItemWidth(w);
	float x = val;
	if (ImGui::InputFloat(base.c_str(), &x)) {
		val = x;
		return true;
	}

	return false;
}

bool boolComponent(float w, std::string base, bool &val) {
	ImGui::PushItemWidth(w);
	bool x = val;
	if (ImGui::Checkbox(base.c_str(), &x)) {
		val = x;
		return true;
	}

	return false;
}

bool stringComponent(float w, std::string base, std::string &val) {
	ImGui::PushItemWidth(w);
	char buf[1024];
	std::fill_n(buf, 1024, 0);
	std::copy_n(val.begin(), (1024 < (int)val.size()) ? 1024 : (int)val.size(), buf);
	if (ImGui::InputText(base.c_str(), buf, 1024)) {
		val = std::string(buf);
		return true;
	}

	return false;
}

bool doubleComponent(float w, std::string base, double &val) {
	ImGui::PushItemWidth(w);
	double x = val;
	if (ImGui::InputDouble(base.c_str(), &x)) {
		val = x;
		return true;
	}
	
	return false;
}

void parseCategory(reflect::TypeDescriptor_Struct::Category &cat, unsigned char *component) {
	ImGuiStyle& style = ImGui::GetStyle();
	float w = ImGui::GetWindowContentRegionMax().x;
	float p = style.FramePadding.x;
	float w2 = (w / 2.0f) - p;
	w2 = (w2 > 30.0f) ? w2 : 30.0f;
	float w3 = (w / 3.0f) - p;
	w3 = (w3 > 30.0f) ? w3 : 30.0f;
	float w4 = (w / 4.0f) - p;
	w4 = (w4 > 30.0f) ? w4 : 30.0f;
	for (auto &member : cat.members) {
		if (member.metadata & reflect::Metadata::ViewInEditor || member.metadata & reflect::Metadata::SetInEditor) {
			ImGui::Text(member.display_name.c_str());
			std::string extended_member_name = std::string("##") + member.variable_name;
			switch (member.type->type) {
			case reflect::TypeDescriptor::ReflectionTypeData::ReflString: {
				std::string &v = *(std::string *)(component + member.offset);
				if (member.metadata & reflect::Metadata::SetInEditor)
					if (stringComponent(w, extended_member_name, v))
						if (member.onChangeCallback)
							member.onChangeCallback(component);
				else
					ImGui::Text("%s", v.c_str());
				break;
			}
			case reflect::TypeDescriptor::ReflectionTypeData::ReflBool: {
				bool &v = *(bool *)(component + member.offset);
				if (member.metadata & reflect::Metadata::SetInEditor)
					if (boolComponent(w, extended_member_name, v))
						if (member.onChangeCallback)
							member.onChangeCallback(component);
				else
					ImGui::Text("%s", v ? "true" : "false");
				break;
			}
			case reflect::TypeDescriptor::ReflectionTypeData::ReflFloat: {
				float &v = *(float *)(component + member.offset);
				if (member.metadata & reflect::Metadata::SetInEditor)
					if (floatComponent(w, extended_member_name, v))
						if (member.onChangeCallback)
							member.onChangeCallback(component);
				else
					ImGui::Text("%f", v);
				break;
			}
			case reflect::TypeDescriptor::ReflectionTypeData::ReflDouble: {
				double &v = *(double *)(component + member.offset);
				if (member.metadata & reflect::Metadata::SetInEditor)
					doubleComponent(w, extended_member_name, v);
				else
					ImGui::Text("%f", v);
				break;
			}
			case reflect::TypeDescriptor::ReflectionTypeData::ReflVec2: {
				glm::vec2 &v = *(glm::vec2 *)(component + member.offset);
				if (member.metadata & reflect::Metadata::SetInEditor)
					if (vec2Component(extended_member_name, v, -1.0))
						if (member.onChangeCallback)
							member.onChangeCallback(component);
				else
					ImGui::Text("%f %f", v.x, v.y);
				break;
			}
			case reflect::TypeDescriptor::ReflectionTypeData::ReflVec3: {
				glm::vec3 &v = *(glm::vec3 *)(component + member.offset);
				if (member.metadata & reflect::Metadata::SetInEditor)
					vec3Component(extended_member_name, v, w3);
				else
					ImGui::Text("%f %f %f", v.x, v.y, v.z);
				break;
			}
			case reflect::TypeDescriptor::ReflectionTypeData::ReflVec4: {
				glm::vec4 &v = *(glm::vec4 *)(component + member.offset);
				if (member.metadata & reflect::Metadata::SetInEditor) {
					if (vec4Component(extended_member_name, v, w4))
						if (member.onChangeCallback)
							member.onChangeCallback(component);
				}
				else
					ImGui::Text("%f %f %f %f", v.x, v.y, v.z, v.w);
				break;
			}
			}
		}
	}

for (auto &c : cat.categories) {
	if (ImGui::TreeNode(c.name.c_str())) {
		parseCategory(c, component);
		ImGui::TreePop();
	}
}
}

void Editor::inspectorPanel() {
	if (show_inspector_panel_) {
		ImGui::Begin("Inspector Panel", &show_inspector_panel_);
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
						ComponentType component_type = ComponentType(i);
						ComponentHandle h = selected_object_->getComponentHandle(component_type);

						if (h != UINT_MAX) {
							reflect::TypeDescriptor_Struct *refl = engine.getSystem(component_type)->getReflection();
							if (refl) {
								Component *component = space->getSubsystem(component_type)->getBaseComponent(h);
								if (ImGui::TreeNode(component_names[i])) {
									if (component_type != COMPONENT_TRANSFORM) {
										if (ImGui::Button("Remove Component")) {
											space->getSubsystem(component_type)->removeComponent(h);
										}
									}

									parseCategory(refl->category, (unsigned char *)component);

									ImGui::TreePop();
								}
							}
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

void Editor::consolePanel() {
	ImGui::Begin("Console", &show_console_);
	ImVec2 winsize = ImGui::GetWindowSize();
	ImGui::BeginChild("Scrolling", ImVec2(0.0f, -24.0f));
	/*for (int i = console_entries_first_; i < console_entries_count_; i = (i + 1) % console_entries_count_) {
		ImGui::Text("%004d: %s", i, console_entries_[i]);
	}*/

	if (console_entries_count_ >= MAX_CONSOLE_ENTRIES) {
		unsigned int i = console_entries_first_;
		do {
			ImGui::Text("%s", console_entries_[i]);
			i = (i + 1) % MAX_CONSOLE_ENTRIES;
		} while (i != console_entries_first_);
	}
	else {
		for (int i = 0; i < console_entries_count_; ++i) {
			ImGui::Text("%s", console_entries_[i]);
		}
	}

	ImGui::EndChild();

	ImGui::PushItemWidth(winsize.x-4.0f);
	if (ImGui::InputText("##Input Command...", console_buffer_, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
		engine.consoleCommand(console_buffer_);
		console_buffer_[0] = 0;
	}
	ImGui::End();
}

void Editor::printConsoleEntry(const char *entry) {
	int n = strlen(entry) + 1;
	char *t = new char[n];
	memcpy(t, entry, n);

	if (console_entries_count_ >= MAX_CONSOLE_ENTRIES) {
		delete console_entries_[console_entries_first_];
		console_entries_[console_entries_first_] = t;
		console_entries_first_ = (console_entries_first_ + 1) % MAX_CONSOLE_ENTRIES;
	}
	else {
		console_entries_[console_entries_count_++] = t;
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

		/*
		if (ImGui::IsMouseHoveringWindow())
			ImGui::OpenPopup("FilePopup");
		if (ImGui::BeginPopup("FilePopup", 1))
		{
			ImGui::Button("Test");
			ImGui::EndPopup();
		}
		*/

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

void Editor::drawGizmos() {
	drawBox(glm::vec3(-2, -2, -2), glm::vec3(2, 2, 2));
}

void Editor::drawBox(glm::vec3 start, glm::vec3 end) {
	auto gw = engine.getGraphicsWrapper();
	
	// gw->DrawImmediateIndexed(0, flase, 0);
}

bool has_left_clicked = false;

void Editor::viewportPanels() {
	bool left_clicking = engine.getInputManager()->GetMouseButton(MOUSE_LEFT) > 0;
	bool right_clicking = engine.getInputManager()->GetMouseButton(MOUSE_RIGHT) > 0;

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

			Camera &cam = camcomp.camera_;
			
			cam.setDirections(fwd, up);
			cam.setPosition(pos);
			cam.setViewport((unsigned int)size.x, (unsigned int)size.y);
			cam.render();

			drawGizmos();

			engine.getGraphicsWrapper()->BindDefaultFramebuffer(false);

			ImTextureID t = (ImTextureID)((GLRenderTarget *)cam.final_buffer_)->getHandle();

			ImGui::GetWindowDrawList()->AddImage(
				t, ImVec2(ImGui::GetCursorScreenPos()),
				ImVec2(ImGui::GetCursorScreenPos().x + size.x, ImGui::GetCursorScreenPos().y + size.y), ImVec2(0, 1), ImVec2(1, 0));

			ImGui::End();
		}
		else {
			bool viewport_selected = false;

			ImVec2 mouse = ImGui::GetMousePos();
			int mousex = (int)mouse.x;
			int mousey = (int)mouse.y;

			double dt = engine.getUpdateTimeDelta();

			int i = 0;
			for (auto &v : viewports_) {
				std::string title = "Viewport ";
				title += std::to_string(++i);

				ImGui::Begin(title.c_str(), &v.viewport_shown);
				ImVec2 size = ImGui::GetWindowSize();
				int sizex = (int)size.x;
				int sizey = (int)size.y;
				/*ImGuiStyle& style = ImGui::GetStyle();
				size.x -= style.FramePadding.x * 2;
				size.y -= style.FramePadding.y * 2;*/

				/*if (v.first) {
					v.first = false;
					v.camera_->setViewport(size.x, size.y);
					//v.camera_->initialize();
				}*/

				//if (viewport_manipulating_) {
				if (!has_left_clicked && left_clicking || right_clicking) {
					ImVec2 min = ImGui::GetWindowPos();
					int minx = (int)min.x;
					int miny = (int)min.y;
					int maxx = minx + sizex;
					int maxy = miny + sizey;

					int midx = int(minx + maxx) / 2;
					int midy = int(miny + maxy) / 2;

					if (viewport_manipulating_ == i) {
						float msensitivity = 0.5f;
						float keymovespeed = 8.0f;
						float cursormovespeed = 2.0f;

						float cx = float(midx - mousex);
						float cy = float(midy - mousey);

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
						// v.setViewMatrix();

						engine.getGraphicsWrapper()->SetCursor(midx, midy);
						viewport_selected = true;
					}
					else if (viewport_manipulating_ == 0) {
						if (mousex < maxx && mousex > minx && mousey > miny && mousey < maxy) {
							if (right_clicking) {
								viewport_manipulating_ = i;

								engine.getGraphicsWrapper()->SetCursor(midx, midy);
							}
							if (left_clicking) {
								RayTraceResults r = v.camera_->rayTraceMousePostion(mousex, mousey);
								if (r.hit) {
									selected_object_handle_ = r.object_handle;
								}
								else {
									selected_object_handle_ = -1;
								}

								/*glm::vec3 eye_pos = v.pos;
								glm::vec3 ray_world;

								if (v.view == Editor::Viewport::Perspective) {
									int cx = mouse.x - min.x;
									int cy = mouse.y - min.y;

									float width = (max.x - min.x);
									float height = (max.y - min.y);

									float x = 0.0f; // (2.0f * cx) / width - 1.0f;
									float y = 0.0f; // (2.0f * cy) / height - 1.0f;
									glm::vec4 ray_clip = glm::vec4(x, y, -1.0, 1.0);
									glm::vec4 ray_eye = glm::inverse(v.camera_->projection_ * v.view_mat) * ray_clip;
									//ray_world = (glm::inverse(v.camera_->view_) * ray_eye);
									// don't forget to normalise the vector at some point
									ray_world = eye_pos + v.fwd * 10.0f; // glm::normalize(ray_world) * 100.0f;
								}
								else {
									ray_world = 10.0f * v.fwd;
								}

								std::cout << "From: " << eye_pos.x << " " << eye_pos.y << " " << eye_pos.z << std::endl;
								std::cout << "To: " << ray_world.x << " " << ray_world.y << " " << ray_world.z << std::endl;
								std::cout << "=========\n";

								btVector3 btRayFrom(eye_pos.x, eye_pos.y, eye_pos.z);
								btVector3 btRayTo(ray_world.x, ray_world.y, ray_world.z);
								btCollisionWorld::ClosestRayResultCallback rayCallback(btRayFrom, btRayTo);
								auto space = engine.getScene(0)->spaces_[0];
								RigidBodySubSystem *sys = ((RigidBodySubSystem *)space->getSubsystem(COMPONENT_RIGID_BODY));
								sys->dynamics_world_->rayTest(btRayFrom, btRayTo, rayCallback);
								if (rayCallback.hasHit()) {
									auto r = rayCallback.m_hitPointWorld;
									auto game_obj_handle = rayCallback.m_collisionObject->getUserIndex();
									auto &game_obj = space->getObject(game_obj_handle);
									
									selected_object_handle_ = game_obj_handle;

									auto s = game_obj.getName().size();
									memset(obj_name + s, 0, 256 - s);
									memcpy(obj_name, game_obj.getName().c_str(), s);
								}
								else {
									selected_object_handle_ = -1;
								*/
							}
						}
					}
				}
				else {
					viewport_manipulating_ = 0;
				}

				v.camera_->setViewport((unsigned int)size.x, (unsigned int)size.y);
				v.camera_->setPosition(v.pos);
				v.camera_->setDirections(v.fwd, v.up);
				v.camera_->render();
				engine.getGraphicsWrapper()->BindDefaultFramebuffer(false);

				ImTextureID t = (ImTextureID)(((GLRenderTarget *)v.camera_->final_buffer_)->getHandle());

				ImGui::GetWindowDrawList()->AddImage(
					t, ImVec2(ImGui::GetCursorScreenPos()),
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

	has_left_clicked = left_clicking;
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

void Editor::writeComponentToJson(reflect::TypeDescriptor_Struct::Category &refl, unsigned char * component_ptr, rapidjson::PrettyWriter<rapidjson::StringBuffer> &w) {
	for (auto &mem : refl.members) {
		std::string n = mem.stored_name;

		unsigned char *p = component_ptr + mem.offset;

		w.Key(n.c_str());

		switch (mem.type->type)
		{
		case reflect::TypeDescriptor::ReflString:
			w.String((*(std::string *)p).c_str());
			break;
		case reflect::TypeDescriptor::ReflBool:
			w.Bool(*(bool *)p);
			break;
		case reflect::TypeDescriptor::ReflInt:
			w.Int(*(int *)p);
			break;
		case reflect::TypeDescriptor::ReflFloat:
			w.Double(*(float *)p);
			break;
		case reflect::TypeDescriptor::ReflDouble:
			w.Double(*(double *)p);
			break;
		case reflect::TypeDescriptor::ReflVec2: {
			glm::vec2 &v = (*(glm::vec2 *)p);
			w.StartArray();
			w.Double(v.x);
			w.Double(v.y);
			w.EndArray();
			break;
		}
		case reflect::TypeDescriptor::ReflVec3: {
			glm::vec3 &v = (*(glm::vec3 *)p);
			w.StartArray();
			w.Double(v.x);
			w.Double(v.y);
			w.Double(v.z);
			w.EndArray();
			break;
		}
		case reflect::TypeDescriptor::ReflVec4: {
			glm::vec4 &v = (*(glm::vec4 *)p);
			w.StartArray();
			w.Double(v.x);
			w.Double(v.y);
			w.Double(v.z);
			w.Double(v.w);
			w.EndArray();
			break;
		}
		}
	}

	for (auto &cat : refl.categories) {
		writeComponentToJson(cat, component_ptr, w);
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
				ComponentType t = (ComponentType)i;
				ComponentHandle h = object.getComponentHandle(t);
				if (h != -1) {
					writer.Key(component_names[i]);
					writer.StartObject();

					reflect::TypeDescriptor_Struct *refl = engine.getSystem(t)->getReflection();
					if (refl) {
						Component *component = space->getSubsystem(t)->getBaseComponent(h);

						writeComponentToJson(refl->category, (unsigned char *)component, writer);
					}
					//space->getSubsystem((ComponentType)i)->writeComponentToJson(h, writer);
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

void Editor::renderSceneGraphTree(std::vector<SceneGraphNode*>& scene_graph_nodes) {
	return;
	
	for (auto &s : scene_graph_nodes) {
		std::string n = engine.getScene(0)->spaces_[0]->getObject(s->object_handle_).getName();

		if (s->children_.size() > 0) {
			ImGui::TreeNode(n.c_str());
			if (ImGui::Button("Select Object")) {
				selected_object_handle_ = s->object_handle_;
				auto s = n.size();
				memset(obj_name + s, 0, 256 - s);
				memcpy(obj_name, n.c_str(), s);
			}
			renderSceneGraphTree(s->children_);
			ImGui::TreePop();
		}
	}
}

void Editor::sceneGraphPanel() {
	if (show_scene_graph_) {
		ImGui::Begin("Scene Graph", &show_scene_graph_);

		if (ImGui::Button("Add GameObject")) {
			engine.getScenes()[0]->spaces_[0]->objects_.emplace_back((GameObjectHandle)engine.getScenes()[0]->spaces_[0]->objects_.size(), "New Object", -1);
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

		// renderSceneGraphTree(scene_graph_);

        ImGui::End();
    }
}

SceneGraphNode::SceneGraphNode(GameObjectHandle o, std::vector<SceneGraphNode*> c) : object_handle_(o), children_(c) {
}

#endif