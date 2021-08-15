#pragma once

#include <string>
#include <map>
#include <fstream>

#include "Shader.hpp"

namespace Grindstone {
	class BaseAssetRenderer;
	class ShaderManager {
		public:
			Shader& LoadShader(BaseAssetRenderer* assetRenderer, const char* path);
		private:
			bool TryGetShader(const char* path, Shader*& shader);
			Shader& CreateShaderFromFile(const char* path);
			void CreateReflectionDataForShader(const char* path, Shader& shader);
			void CreateShaderGraphicsPipeline(const char* path, Shader& shader);
		private:
			std::map<std::string, Shader> shaders;
	};
}
