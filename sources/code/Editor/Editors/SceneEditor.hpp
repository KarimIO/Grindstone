#pragma once

#include <string>
#include "../IEditor.hpp"

namespace Grindstone {
    namespace Editor {
        class SceneEditor : public Grindstone::Editor::IEditor {
        public:
            SceneEditor();
            virtual bool initialize() override;
            virtual void update() override;
        private:
        };
    }
}