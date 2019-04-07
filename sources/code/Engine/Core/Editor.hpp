#ifndef _EDITOR_H
#define _EDITOR_H

#include "../Core/Camera.hpp"

class ImguiManager;

class Editor {
public:
    Editor(ImguiManager *manager);
    void update();
private:
	struct Viewport {
		Camera *camera_;
		enum View : unsigned int {
			Top = 0,
			Bottom,
			Left,
			Right,
			Front,
			Back,
			Perspective
		} view;
		glm::vec3 pos;
		glm::vec3 fwd;
		glm::vec3 right;
		glm::vec3 up;
		glm::mat4 view_mat;
		glm::vec3 angles;

		bool first;
		const char *view_option = nullptr;
		const char *debug_combo_option = nullptr;
		Viewport(Camera *c, View v);
		void calcDirs();
		void setViewMatrix();
		void setView(View v);
	};
	std::vector<Viewport> viewports_;

    ImguiManager *manager_;

	GameObjectHandle selected_object_handle_;

	bool show_scene_graph_;
    bool show_viewport_;
    bool show_component_panel_;
    bool show_asset_browser_;
    std::string asset_path_;
    std::string next_asset_path_;

	void prepareDockspace();
    void sceneGraphPanel();
    void componentPanel();
    void assetPanel();
	void viewportPanels();

    void getDirectory();

	void displayComponent(ComponentType type, ComponentHandle handle);

    struct FileElement {
        std::string path;
        std::string name;
        FileElement(std::string p, std::string n) : path(p), name(n) {}
    };
    std::vector<FileElement> directories_;
    std::vector<FileElement> files_;

	char *obj_name;
	unsigned int viewport_manipulating_;
};

#endif