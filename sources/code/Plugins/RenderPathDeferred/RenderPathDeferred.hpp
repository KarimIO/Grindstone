#pragma once

#include <Common/Rendering/RenderPathBase.hpp>

namespace Grindstone {
    namespace Rendering {
        class RenderPathDeferred : public RenderPathBase {
        public:
            virtual ~RenderPathDeferred() override;
            virtual void render() override;
            virtual void resizeViewport(uint32_t x, uint32_t y) override;
        }; // class RenderPathBase
    } // namespace Rendering
} // namespace Grindstone
