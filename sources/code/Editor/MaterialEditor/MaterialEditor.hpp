#pragma once

#include <Editor/CoreEditor/EditorInstance.hpp>
#include <string>

struct ImGuiWindowClass;

namespace Grindstone {
	class EditorManager;

	class MaterialEditor : public EditorInstance {
	public:
		MaterialEditor(std::string path, unsigned int id, EditorManager* manager);
		virtual ~MaterialEditor() override;
	public:
		bool initialize();
		virtual void run() override;
		virtual void cleanup() override;
		virtual unsigned int getID() const override;
	private:
		bool prepareDockspace(const char* name);
	private:
		std::string material_name_;
		std::string material_path_;
		unsigned int id_;

		bool should_show_viewport;
		bool should_show_properties;

		ImGuiWindowClass* imgui_window_class_;
		EditorManager* manager_;
	};
}