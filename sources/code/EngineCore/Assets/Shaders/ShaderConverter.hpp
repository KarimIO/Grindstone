#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include "Shader.hpp"

namespace Grindstone {
	namespace ShaderConverter {
		enum class SourceType {
			VulkanGlsl,
			// VulkanSpirv,
			// OpenglGlsl,
			// OpenglSpirv
		};
		
		void ConvertShader(const char* inputPath, SourceType sourceType);

		namespace FromVulkanGlsl {
			void ConvertShader(const char* inputPath, std::vector<char>& inputGlsl);
			void ConvertShaderReflectionInfo(const char* inputPath, std::vector<char>& inputGlsl);
			void ConvertShaderToOpenglSpirv(const char* inputPath, std::vector<char>& inputGlsl);
			void ConvertShaderToOpenglGlsl(const char* inputPath, std::vector<char>& inputGlsl);
			void ConvertShaderToVulkanSpirv(const char* inputPath, std::vector<char>& inputGlsl);
			void ConvertShaderToHlsl(const char* inputPath, std::vector<char>& inputGlsl);
		}
	};
}
