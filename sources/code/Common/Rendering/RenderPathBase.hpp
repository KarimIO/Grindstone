 #pragma once

#include <stdint.h>

namespace Grindstone {
    namespace Rendering {
        class RenderPathBase {
        public:
            virtual ~RenderPathBase() {};
            virtual void render() = 0;
            virtual void resizeViewport(uint32_t x, uint32_t y) = 0;
        }; // class RenderPathBase
    } // namespace Rendering
} // namespace Grindstone
