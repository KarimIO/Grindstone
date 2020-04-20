#include <Engine/UI/UICanvas.hpp>

void UINode::initialize() {
}

void UINode::setLayout(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
    layout_updated_ = LayoutUpdateStatus::NeedsUpdate;
    
    if (parent_) {
        updateLayoutParent();
    }
}

void UINode::traverseLayout() {
    layout_updated_ = LayoutUpdateStatus::NoUpdate;
    
    for (auto &n : children_) {
        if (n->layout_updated_ == LayoutUpdateStatus::NeedsUpdate) {
            n->updateLayout(top_left_actual_, bottom_right_actual_);
        }
        else if (n->layout_updated_ == LayoutUpdateStatus::ChildNeedsUpdate) {
            n->traverseLayout();
        }
    }
}

void UINode::updateLayout(Vector2u top_left, Vector2u bottom_right) {
    Vector2i top_left_offset_;
    Vector2i bottom_right_offset_;
    
    Vector2f top_left_normalized_;
    Vector2f bottom_right_normalized_;
    
    Vector2f pivot_;

    Vector2u top_left_actual_;
    Vector2u bottom_right_actual_;

    unsigned int width  = bottom_right.x - top_left.x;
    unsigned int height = bottom_right.y - top_left.y;

    top_left_actual_.x = top_left.x + top_left_offset_.x + (width * top_left_normalized_.x);
    top_left_actual_.y = top_left.y + top_left_offset_.y + (height * top_left_normalized_.y);
    bottom_right_actual_.x = top_left.x + top_left_offset_.x + (width * top_left_normalized_.x);
    bottom_right_actual_.y = top_left.y + top_left_offset_.y + (height * top_left_normalized_.y);
    
    for (auto &n : children_) {
        n->updateLayout(top_left, bottom_right);
    }
}

void UINode::updateLayoutParent() {
    if (layout_updated_ == LayoutUpdateStatus::NoUpdate) {
        layout_updated_ = LayoutUpdateStatus::ChildNeedsUpdate;
    }

    if (parent_) {
        // Non-root node
        parent_->updateLayoutParent();
    }
}

UINode::~UINode() {
    for (auto& n : children_) {
        delete n;
    }
}