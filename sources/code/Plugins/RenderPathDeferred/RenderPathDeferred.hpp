#pragma once

#include <Common/Rendering/RenderPathBase.hpp>

namespace Grindstone {
    namespace Rendering {
        class RenderPathDeferred : public RenderPathBase {
        public:
            enum class Passes {
                Unlit,
                Opaque,
                Transparency
            };

            virtual ~RenderPathDeferred() override;
            virtual void initialize() override;
            virtual void render() override;
            void renderGeometryQueueOpaque(uint32_t id);
            void renderGeometryQueueForward(uint32_t id);
            virtual void resizeViewport(uint32_t x, uint32_t y) override;
        }; // class RenderPathBase
    } // namespace Rendering
} // namespace Grindstone
