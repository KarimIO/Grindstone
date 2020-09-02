#pragma once

#include <vector>
#include "Material.hpp"
#include <Common/Graphics/Pipeline.hpp>

namespace Grindstone {
    namespace Renderer {
        struct Pipeline {
            GraphicsAPI::Pipeline* pipeline_;
            std::vector<Material> materials_;
        }; // struct Pipeline
	} // namespace Renderer
} // namespace Grindstone
