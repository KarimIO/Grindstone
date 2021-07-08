#pragma once

#include <string>
#include <map>
#include <fstream>

#include "Shader.hpp"

namespace Grindstone {
	class ShaderManager {
		public:
			Shader& LoadShader(const char* path);
			bool TryGetShader(const char* path, Shader*& shader);
		private:
			Shader& CreateShaderFromFile(const char* path);
			void CreateReflectionDataForShader(const char* path, Shader& shader);
			void CreateShaderGraphicsPipeline(const char* path, Shader& shader);
		private:
			std::map<std::string, Shader> shaders;
	};
}
