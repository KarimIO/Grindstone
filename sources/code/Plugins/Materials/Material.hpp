#pragma once

#include <string>

#include <Common/Graphics/Texture.hpp>
#include <Common/Graphics/UniformBuffer.hpp>

namespace Grindstone {
	struct Material {
	public:
		Grindstone::GraphicsAPI::TextureBinding *texture_binding_;
		Grindstone::GraphicsAPI::UniformBuffer *parameters_;
		char *param_buffer_;
	private:
		std::string path_;
	};
} // namespace Grindstone
