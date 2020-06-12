#pragma once

#include <Engine/UI/UINode.hpp>

class UICanvas {
public:
    virtual void initialize();
    virtual void update();
    virtual void setRoot(UINode*);
    virtual UINode* getRoot();
    ~UICanvas();
private:
    UINode *root_node_ = nullptr;
    bool canvas_layout_updated_;

    Vector2u position_;
    Vector2u size_;
};
