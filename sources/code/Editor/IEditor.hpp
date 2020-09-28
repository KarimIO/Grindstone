#pragma once

#include <string>

namespace Grindstone {
    namespace Editor {
        class IEditor {
        public:
            virtual bool initialize() = 0;
            virtual void update() = 0;
        };
    }
}