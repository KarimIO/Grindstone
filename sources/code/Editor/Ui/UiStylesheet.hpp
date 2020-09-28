#pragma once

#include <vector>

namespace Grindstone {
    namespace Ui {
        class Stylesheet {
        public:
            struct Ruleset {
                struct Selector {
                };
                
                struct Declaration {
                    const char* property_;
                    const char* value_;
                };
            };
        public:
            bool initialize();
            void buildRenderTree();
            void buildLayout();
            void preparePaint();
        private:
            std::vector<Rule> ruleet;
        };
    }
}