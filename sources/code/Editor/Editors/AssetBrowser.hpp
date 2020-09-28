#pragma once

#include <string>
#include "../IEditor.hpp"

namespace Grindstone {
    namespace Editor {
        class AssetBrowser : public Grindstone::Editor::IEditor {
        public:
            AssetBrowser();
            virtual bool initialize() override;
            virtual void update() override;
        private:
            std::string path_;
        };
    }
}