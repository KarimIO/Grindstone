#include "MaterialEditor.hpp"
#include <CoreEditor/EditorManager.hpp>

namespace Grindstone {
	bool MaterialEditor::prepareDockspace(const char* name) {
		return false;
		//static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;

		//// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		//// because it would be confusing to have two docking targets within each others.
		//ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;

		//// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		//if (opt_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		//	window_flags |= ImGuiWindowFlags_NoBackground;

		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		//ImGui::SetNextWindowClass(manager_->getMainImguiWindowClass());
		//bool win_open = ImGui::Begin(name, nullptr, window_flags);
		//ImGui::PopStyleVar();
		//if (win_open) {
		//	opt_flags |= ImGuiDockNodeFlags_None;

		//	// Dockspace
		//	ImGuiIO& io = ImGui::GetIO();
		//	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		//	{
		//		ImGuiID dockspace_id = ImGui::GetID(name);
		//		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags, imgui_window_class_);
		//	}

		//	if (ImGui::BeginMenuBar()) {
		//		if (ImGui::BeginMenu("File")) {
		//			/*if (ImGui::MenuItem("Save", "", false))
		//				saveFile();
		//			if (ImGui::MenuItem("Save As", "", false))
		//				saveFileAs();
		//			ImGui::Separator();
		//			if (ImGui::MenuItem("Load", "", false))
		//				loadFile();
		//			if (ImGui::MenuItem("Load From", "", false))
		//				loadFileFrom();
		//			ImGui::Separator();
		//			if (ImGui::MenuItem("Import", "", false))
		//				importFile();
		//			ImGui::Separator();
		//			if (ImGui::MenuItem("Close", "", false))
		//				engine.shutdown();*/
		//			ImGui::EndMenu();
		//		}

		//		if (ImGui::BeginMenu("Window")) {
		//			ImGui::MenuItem("Show Viewport", "", &should_show_viewport);
		//			ImGui::MenuItem("Show Properties", "", &should_show_properties);
		//			ImGui::EndMenu();
		//		}

		//		if (ImGui::BeginMenu("Build")) {
		//			ImGui::EndMenu();
		//		}

		//		/*if (!engine.edit_is_simulating_) {
		//			if (ImGui::BeginMenu("Simulate")) {
		//				engine.startSimulation();
		//				ImGui::EndMenu();
		//			}
		//		}
		//		else {
		//			if (ImGui::BeginMenu("Stop Simulation")) {
		//				engine.stopSimulation();
		//				ImGui::EndMenu();
		//			}
		//		}*/

		//		ImGui::EndMenuBar();
		//	}
		//}

		//ImGui::End();

		//return win_open;
	}

	MaterialEditor::MaterialEditor(std::string path, unsigned int id, EditorManager* manager) : id_(id), manager_(manager) {
		//material_name_ = material_path_ = path;
		//should_show_properties = true;
		//should_show_viewport = true;

		///*ImGuiWindowClass
		//{
		//	ImGuiID             ClassId;                    // User data. 0 = Default class (unclassed). Windows of different classes cannot be docked with each others.
		//	ImGuiID             ParentViewportId;           // Hint for the platform back-end. If non-zero, the platform back-end can create a parent<>child relationship between the platform windows. Not conforming back-ends are free to e.g. parent every viewport to the main viewport or not.
		//	ImGuiViewportFlags  ViewportFlagsOverrideSet;   // Viewport flags to set when a window of this class owns a viewport. This allows you to enforce OS decoration or task bar icon, override the defaults on a per-window basis.
		//	ImGuiViewportFlags  ViewportFlagsOverrideClear; // Viewport flags to clear when a window of this class owns a viewport. This allows you to enforce OS decoration or task bar icon, override the defaults on a per-window basis.
		//	bool                DockingAlwaysTabBar;        // Set to true to enforce single floating windows of this class always having their own docking node (equivalent of setting the global io.ConfigDockingAlwaysTabBar)
		//	bool                DockingAllowUnclassed;      // Set to true to allow windows of this class to be docked/merged with an unclassed window.

		//	ImGuiWindowClass() { ClassId = 0; ParentViewportId = 0; ViewportFlagsOverrideSet = ViewportFlagsOverrideClear = 0x00; DockingAlwaysTabBar = false; DockingAllowUnclassed = true; }
		//};*/

		//imgui_window_class_ = new ImGuiWindowClass();
		//imgui_window_class_->ClassId = id;
	}

	MaterialEditor::~MaterialEditor() {
	}

	bool MaterialEditor::initialize() {
		//auto space = engine.addSpace();
		//auto scene = space.addScene();
		//auto object = space.addObject();
		//object.addComponent(TRANSFORM_COMPONENT);

		return false;
	}

	void MaterialEditor::run() {
		/*if (prepareDockspace(material_name_.c_str())) {
			if (should_show_viewport) {
				ImGui::SetNextWindowClass(imgui_window_class_);
				if (ImGui::Begin((material_name_ + " Viewport").c_str(), &should_show_viewport)) {

				}
				ImGui::End();
			}
			if (should_show_properties) {
				ImGui::SetNextWindowClass(imgui_window_class_);
				if (ImGui::Begin((material_name_ + " Properties").c_str(), &should_show_properties)) {
					char name_buff[256];
					name_buff[0] = 0;
					ImGui::InputText("Name", name_buff, 256);
				}
				ImGui::End();
			}
		}*/
	}

	void MaterialEditor::cleanup() {
	}

	unsigned int MaterialEditor::getID() const {
		return id_;
	}
}