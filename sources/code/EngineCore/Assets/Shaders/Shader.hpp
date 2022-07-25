#pragma once

#include <string>
#include <vector>
#include "Common/Graphics/Pipeline.hpp"
#include "ShaderReflectionData.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	struct Material;
	struct Shader : public Asset {
		std::string basePath;
		GraphicsAPI::Pipeline* pipeline = nullptr;
		GraphicsAPI::TextureBindingLayout* textureBindingLayout = nullptr;
		ShaderReflectionData reflectionData;
		std::vector<Material*> materials;
	};
}
