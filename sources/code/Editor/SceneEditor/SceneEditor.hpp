#pragma once

#include <Editor/CoreEditor/EditorInstance.hpp>
#include <string>

struct ImGuiWindowClass;

namespace Grindstone {
	class EditorManager;

	class SceneEditor : public EditorInstance {
	public:
		SceneEditor(unsigned int id, EditorManager* manager);
		virtual ~SceneEditor() override;
	public:
		void newScene();
		void openScene(std::string path);
		virtual void run() override;
		virtual void cleanup() override;
		virtual unsigned int getID() const override;
	private:
		bool prepareDockspace(const char* name);
		void sceneGraphPanel();
		void inspectorPanel();
		void assetPanel();
		void logPanel();
		void viewportPanels();
	private:
		std::string scene_name_;
		std::string scene_path_;
		unsigned int id_;
		std::string id_as_str_;

		bool should_show_scene_graph_;
		bool should_show_inspector_;
		bool should_show_asset_browser_;
		bool should_show_log_;

		std::string panel_name_scene_graph_;
		std::string panel_name_inspector_;
		std::string panel_name_asset_browser_;
		std::string panel_name_log_;

		ImGuiWindowClass* imgui_window_class_;
		EditorManager* manager_;
	};
}