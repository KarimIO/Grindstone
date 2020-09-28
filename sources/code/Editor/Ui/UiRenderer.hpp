#pragma once

namespace Grindstone {
    namespace Ui {
        class Document;
        class Stylesheet;

        class Renderer {
        public:
            bool initialize();
            void buildRenderTree();
            void buildLayout();
            void preparePaint();
        private:
            Ui::Document* dom_;
            Ui::Stylesheet* cssom_;
        };
    }
}