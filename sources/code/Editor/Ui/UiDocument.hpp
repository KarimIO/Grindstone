#pragma once

#include "UiNode.hpp"

namespace Grindstone {
    namespace Ui {
        class Document {
        public:
            bool initialize();
            Node* createPanel(const char* tag, Node* parent);
            Node* createText(const char* text, Node* parent);
            void setRoot(Node* node);
        private:
            Ui::Node* root_;
        };
    }
}