#pragma once

#include <string>

#include <Common/Graphics/Texture.hpp>
#include <Common/Graphics/UniformBuffer.hpp>

namespace Grindstone {
    namespace Renderer {
		struct Material {
		public:
			Grindstone::GraphicsAPI::TextureBinding *texture_binding_;
			Grindstone::GraphicsAPI::UniformBuffer *parameters_;
			char *param_buffer_;
		private:
			std::string path_;
		}; // struct Material
	} // namespace Renderer
} // namespace Grindstone
