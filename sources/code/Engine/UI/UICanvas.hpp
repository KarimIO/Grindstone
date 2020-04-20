#pragma once

#include <Engine/UI/UINode.hpp>

class UICanvas {
public:
    void initialize();
    void update();
    void setRoot(UINode*);
    UINode* getRoot();
    ~UICanvas();
private:
    UINode *root_node_;
    bool canvas_layout_updated_;

    Vector2u position_;
    Vector2u size_;
};
