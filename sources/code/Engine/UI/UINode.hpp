#pragma once

#include <vector>
#include <Engine/UI/Constraints.hpp>

enum class LayoutUpdateStatus {
    NoUpdate = 0,
    NeedsUpdate,
    ChildNeedsUpdate
};

struct Vector2i {
    int x;
    int y;
};

struct Vector2u {
    unsigned int x;
    unsigned int y;
};

struct Vector2f {
    float x;
    float y;
};

class UINode {
public:
    void initialize();
    
    void setLayout(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
    void traverseLayout();
    void updateLayout(Vector2u top_left, Vector2u bottom_right);
    void updateLayoutParent();

    void draw();
    ~UINode();
private:
    std::vector<UINode *> children_;
    UINode *parent_;
    LayoutUpdateStatus layout_updated_;

    Vector2i top_left_offset_;
    Vector2i bottom_right_offset_;
    
    Vector2f top_left_normalized_;
    Vector2f bottom_right_normalized_;
    
    Vector2f pivot_;

    Vector2u top_left_actual_;
    Vector2u bottom_right_actual_;
};
