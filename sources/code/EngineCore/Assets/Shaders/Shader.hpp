#pragma once

#include <string>
#include <vector>
#include "Common/Graphics/Pipeline.hpp"
#include "ShaderReflectionData.hpp"

namespace Grindstone {
	struct Material;
	struct Shader {
		std::string basePath;
		GraphicsAPI::Pipeline* pipeline = nullptr;
		GraphicsAPI::TextureBindingLayout* textureBindingLayout = nullptr;
		ShaderReflectionData reflectionData;
		std::vector<Material*> materials;
	};
}
