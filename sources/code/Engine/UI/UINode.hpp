#pragma once

#include <vector>
#include <Engine/UI/Constraints.hpp>

enum class LayoutUpdateStatus {
    NoUpdate = 0,
    NeedsUpdate,
    ChildNeedsUpdate
};

struct Vector2i {
    Vector2i();
    Vector2i(int _x, int _y);
    int x;
    int y;
};

struct Vector2u {
    Vector2u();
    Vector2u(unsigned int _x, unsigned int _y);
    unsigned int x;
    unsigned int y;
};

struct Vector2f {
    Vector2f();
    Vector2f(float _x, float _y);
    float x;
    float y;
};

class UINode {
public:
    UINode();
    void initialize();
    
    void setLayout(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
    void traverseLayout();
    void updateLayout(Vector2u top_left, Vector2u bottom_right);
    void updateLayoutParent();

    void draw();
    ~UINode();
protected:
    std::vector<UINode *> children_;
    UINode *parent_;
    LayoutUpdateStatus layout_updated_;

    float top_;
    float left_;
    float bottom_;
    float right_;

    UiConstraint constraint_top_;
    UiConstraint constraint_left_;
    UiConstraint constraint_bottom_;
    UiConstraint constraint_right_;
};
