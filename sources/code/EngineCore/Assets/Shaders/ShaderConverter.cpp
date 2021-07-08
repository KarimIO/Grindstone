#include "EngineCore/Utils/Utilities.hpp"
#include "ShaderConverter.hpp"
using namespace Grindstone;

// Idea: Convert files to VulkanGlsl first before converting to other formats
void ShaderConverter::ConvertShader(const char* inputPath, SourceType sourceType) {
	auto fileData = Utils::LoadFile(inputPath);

	switch(sourceType) {
		case SourceType::VulkanGlsl:
			FromVulkanGlsl::ConvertShader(inputPath, fileData);
			break;
		default:
			// Fault
			break;
	}
}

void ShaderConverter::FromVulkanGlsl::ConvertShader(const char* inputPath, std::vector<char>& inputGlsl) {
	ConvertShaderToOpenglSpirv(inputPath, inputGlsl);
	ConvertShaderToOpenglGlsl(inputPath, inputGlsl);
	ConvertShaderToVulkanSpirv(inputPath, inputGlsl);
	ConvertShaderToHlsl(inputPath, inputGlsl);
}

void ShaderConverter::FromVulkanGlsl::ConvertShaderReflectionInfo(
	const char* inputPath,
	std::vector<char>& inputGlsl
) {

}

void ShaderConverter::FromVulkanGlsl::ConvertShaderToOpenglSpirv(
	const char* inputPath,
	std::vector<char>& inputGlsl
) {

}

void ShaderConverter::FromVulkanGlsl::ConvertShaderToOpenglGlsl(
	const char* inputPath,
	std::vector<char>& inputGlsl
) {

}

void ShaderConverter::FromVulkanGlsl::ConvertShaderToVulkanSpirv(
	const char* inputPath,
	std::vector<char>& inputGlsl
) {

}

void ShaderConverter::FromVulkanGlsl::ConvertShaderToHlsl(
	const char* inputPath,
	std::vector<char>& inputGlsl
) {

}
