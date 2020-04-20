#include <Engine/UI/UICanvas.hpp>

void UICanvas::initialize() {

}

void UICanvas::update() {
    if (canvas_layout_updated_) {
        root_node_->updateLayout(position_, size_);
    }
    else {
        root_node_->traverseLayout();
    }
}

void UICanvas::setRoot(UINode* node) {
    root_node_ = node;
}

UINode* UICanvas::getRoot() {
    return root_node_;
}

UICanvas::~UICanvas() {
    delete root_node_;
}