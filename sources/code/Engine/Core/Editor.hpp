#ifndef _EDITOR_H
#define _EDITOR_H

#include "../Core/Camera.hpp"
#include <vector>

#define MAX_CONSOLE_ENTRIES 256

class ImguiManager;

struct SceneGraphNode {
	GameObjectHandle object_handle_;
	std::vector<SceneGraphNode *> children_;
	SceneGraphNode(GameObjectHandle o, std::vector<SceneGraphNode *> c);
};

class Editor {
public:
	void refreshSceneGraph();
	void setPath(std::string path);
	Editor(ImguiManager *manager);
    void update();
	void reload(ImguiManager *manager);

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

		bool viewport_shown;
		bool first;
		const char *view_option = nullptr;
		const char *debug_combo_option = nullptr;
		Viewport(Camera *c, View v);
		void calcDirs();
		void setViewMatrix();
		void setView(View v);
	};
	std::vector<Viewport> viewports_;

	void printConsoleEntry(const char * entry);

	ImguiManager *manager_;
private:

	double next_refresh_asset_directory_time_;

	std::string path_;

	GameObjectHandle selected_object_handle_;

	bool show_scene_graph_;
    bool show_viewport_;
	bool show_console_;
    bool show_inspector_panel_;
    bool show_asset_browser_;
    std::string asset_path_;
    std::string next_asset_path_;

	void prepareDockspace();
    void sceneGraphPanel();
    void inspectorPanel();
	void consolePanel();
	void assetPanel();
	void drawGizmos();
	void drawBox(glm::vec3 start, glm::vec3 end);
	void viewportPanels();

	void saveFile();
	void saveFileAs();
	void writeComponentToJson(reflect::TypeDescriptor_Struct::Category &refl, unsigned char * componentPtr, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
	void saveFile(std::string path, Scene * scene);
	void cleanCameras();
	void importFile();
	void loadFileFrom();
	void loadFile();
	void renderSceneGraphTree(std::vector<SceneGraphNode *> &scene_graph_nodes);

    void getDirectory();

    struct FileElement {
        std::string path;
        std::string name;
        FileElement(std::string p, std::string n) : path(p), name(n) {}
    };
    std::vector<FileElement> directories_;
    std::vector<FileElement> files_;

	std::vector<SceneGraphNode *> scene_graph_;

	char *obj_name;
	char console_buffer_[256];
	char *console_entries_[MAX_CONSOLE_ENTRIES];
	unsigned int console_entries_first_;
	unsigned int console_entries_count_;
	unsigned int viewport_manipulating_;
};

#endif