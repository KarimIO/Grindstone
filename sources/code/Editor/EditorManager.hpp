#pragma once

#include "EditorRenderer.hpp"

namespace Grindstone {
    namespace Editor {
        class IEditor;

        class Manager {
        public:
            bool initialize();
            void run();

            void loadPlugin();

            Renderer renderer_;

        private:
            bool loadEngine();
            bool addDefaultPlugins();
            bool createDefaultEditors();

        private:
            bool is_running_;
            std::vector<Editor::IEditor* > editors_;
        };
    }
}