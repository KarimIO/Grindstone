#pragma once

#include <vector>

namespace Grindstone {
    namespace Ui {
        struct Node;
        class Document;
        class Stylesheet;
        class Renderer;

        class RenderTree {
        public:
            struct Node {
                struct Layout {
                    float left_;
                    float right_;
                    float up_;
                    float down_;
                } layout_;
                std::vector<Node* > children_;
            }; // Node
        public:
            bool initialize(Ui::Document& doc);
            Ui::RenderTree::Node* initializeNode(Ui::Node* node, Ui::RenderTree::Node* rtNodeParent);
			void render();
			void renderNode(Ui::RenderTree::Node* rtNode);
            void buildRenderTree();
            void buildLayout();
            void preparePaint();
        private:
            Ui::Document* dom_;
            Ui::Stylesheet* cssom_;
            Ui::RenderTree::Node* root_node_;
            Ui::Renderer* renderer_;
        };
    }
}