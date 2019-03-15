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
	bool show_assets_;

	void prepareDockspace();
    void sceneGraphPanel();
    void componentPanel();
    void assetPanel();
	void viewportPanels();
};

#endif