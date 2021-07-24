#pragma once

#include <string>
#include "EngineCore/Assets/Shaders/Shader.hpp"

namespace Grindstone {
	struct Material {
		std::string name;
		std::string shaderPath;
		Shader* shader;
		GraphicsAPI::TextureBinding* textureBinding;
		GraphicsAPI::UniformBufferBinding* uniformBufferBinding;
		GraphicsAPI::UniformBuffer* uniformBufferObject;
		char* buffer;

		/*~Material() {
			delete buffer;
		}*/
	};
}
