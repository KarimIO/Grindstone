#pragma once

#include <string>
#include "Common/Graphics/Pipeline.hpp"
#include "ShaderReflectionData.hpp"

namespace Grindstone {
	struct Shader {
		std::string basePath;
		GraphicsAPI::Pipeline* pipeline = nullptr;
		ShaderReflectionData reflectionData;
	};
}
