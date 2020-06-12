#include "SceneEditor.hpp"
#include <CoreEditor/EditorManager.hpp>
//#include <dear_imgui/imgui.h>
//#include <dear_imgui/imgui_internal.h>

namespace Grindstone {
	bool SceneEditor::prepareDockspace(const char* name) {
		return false;
		/*static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiDockNodeFlags_DockSpace;
		
		ImGuiID dockspace_id = ImGui::GetID(name);
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		if (ImGui::DockBuilderGetNode(dockspace_id) == NULL) {
			ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
			ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

			ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
			ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
			ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.20f, NULL, &dock_main_id);
			ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);

			ImGui::DockBuilderDockWindow(panel_name_asset_browser_.c_str(), dock_id_bottom);
			ImGui::DockBuilderDockWindow(panel_name_log_.c_str(), dock_id_bottom);
			ImGui::DockBuilderDockWindow(panel_name_scene_graph_.c_str(), dock_id_left);
			ImGui::DockBuilderDockWindow(panel_name_inspector_.c_str(), dock_id_right);

			std::string title = "Viewport##0_" + id_as_str_;
			ImGui::DockBuilderDockWindow(title.c_str(), dock_main_id);
			ImGui::DockBuilderFinish(dockspace_id);
		}

		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowClass(manager_->getMainImguiWindowClass());
		bool win_open = ImGui::Begin(name, nullptr, window_flags);
		ImGui::PopStyleVar();
		
		if (win_open) {
			// Dockspace
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags, imgui_window_class_);
			}

			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Window")) {
					ImGui::MenuItem("Show Asset Browser",	"", &should_show_asset_browser_);
					ImGui::MenuItem("Show Log",				"", &should_show_log_);
					ImGui::MenuItem("Show Scene Graph",		"", &should_show_scene_graph_);
					ImGui::MenuItem("Show Inspector",		"", &should_show_inspector_);

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Build")) {
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
		}
		
		ImGui::End();
		return win_open;*/
	}

	SceneEditor::SceneEditor(unsigned int id, EditorManager* manager) : id_(id), manager_(manager) {
		/*
		should_show_scene_graph_ = true;
		should_show_inspector_ = true;
		should_show_asset_browser_ = true;
		should_show_log_ = true;

		id_as_str_ = std::to_string(id_);
		panel_name_scene_graph_ = "Scene Graph##" + id_as_str_;
		panel_name_inspector_ = "Inspector##" + id_as_str_;
		panel_name_asset_browser_ = "Asset Browser##" + id_as_str_;
		panel_name_log_ = "Log##" + id_as_str_;

		imgui_window_class_ = new ImGuiWindowClass();
		imgui_window_class_->ClassId = id_;
		*/
	}

	void SceneEditor::newScene() {
		scene_name_ = "New Scene";
		scene_path_ = "";
	}

	void SceneEditor::openScene(std::string path) {
		scene_name_ = path;
		scene_path_ = path;
	}

	SceneEditor::~SceneEditor() {
	}

	void SceneEditor::run() {
		if (prepareDockspace(scene_name_.c_str())) {
			sceneGraphPanel();
			inspectorPanel();
			assetPanel();
			logPanel();
			viewportPanels();
		}
	}

	void SceneEditor::cleanup() {
	}

	unsigned int SceneEditor::getID() const {
		return id_;
	}

	void SceneEditor::sceneGraphPanel() {
		/*
		if (should_show_scene_graph_) {
			ImGui::SetNextWindowClass(imgui_window_class_);
			if (ImGui::Begin(panel_name_scene_graph_.c_str(), &should_show_scene_graph_)) {
				ImGui::Text("Scene Graph");
			}
			ImGui::End();
		}
		*/
	}

	void SceneEditor::inspectorPanel() {
	}

	void SceneEditor::assetPanel() {
	}

	void SceneEditor::logPanel() {
	}

	void SceneEditor::viewportPanels() {
		std::vector<int> viewports_(1);
		unsigned int i = 0;
		for (auto& v : viewports_) {
		}
	}
}