#ifndef _EDITOR_H
#define _EDITOR_H

#include "../Core/Camera.hpp"

class ImguiManager;

class Editor {
public:
    Editor(ImguiManager *manager);
    void update();
private:
    ImguiManager *manager_;

	Camera *cameras_;
	GameObject *selected_object_;

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

    struct FileElement {
        std::string path;
        std::string name;
        FileElement(std::string p, std::string n) : path(p), name(n) {}
    };
    std::vector<FileElement> directories_;
    std::vector<FileElement> files_;
};

#endif