#pragma once

#include <string>
#include "EngineCore/ECS/Entity.hpp"
#include "Common/ResourcePipeline/Uuid.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class UniformBufferBinding;
		class TextureBinding;
		class UniformBuffer;
	}

	struct Shader;
	struct Material {
		std::string name;
		Uuid uuid;
		std::string shaderPath;
		Shader* shader;
		GraphicsAPI::TextureBinding* textureBinding;
		GraphicsAPI::UniformBufferBinding* uniformBufferBinding;
		GraphicsAPI::UniformBuffer* uniformBufferObject;
		char* buffer;
		std::vector<std::pair<ECS::Entity, void*>> renderables;
	};
}
