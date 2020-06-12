#include <Engine/UI/UICanvas.hpp>
#include <Engine/UI/UIPanel.hpp>

void UICanvas::initialize() {
    UIPanel* root = new UIPanel("../assets/materials/ui.gmat");
    setRoot(root);

    root->setLayout(0, 0, 1920, 1080);
}

void UICanvas::update() {
    if (root_node_) {
        if (canvas_layout_updated_) {
            root_node_->updateLayout(position_, size_);
        }
        else {
            root_node_->traverseLayout();
        }
    }
}

void UICanvas::setRoot(UINode* node) {
    root_node_ = node;
}

UINode* UICanvas::getRoot() {
    return root_node_;
}

UICanvas::~UICanvas() {
    if (root_node_)
        delete root_node_;
}