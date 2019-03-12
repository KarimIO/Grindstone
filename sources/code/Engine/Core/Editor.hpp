#ifndef _EDITOR_H
#define _EDITOR_H

class ImguiManager;

class Editor {
public:
    Editor(ImguiManager *manager);
    void update();
private:
    ImguiManager *manager_;

    bool show_scene_graph_;

    void sceneGraphPanel();
    void componentPanel();
    void assetPanel();
};

#endif