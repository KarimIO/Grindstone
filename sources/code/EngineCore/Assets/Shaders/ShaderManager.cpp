#include <filesystem>
#include "ShaderManager.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Common/Graphics/Core.hpp"
#include "ShaderReflectionLoader.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

std::string GetShaderPath(const char* basePath, ShaderStage shaderStage, GraphicsAPI::Core* core) {
	const char* shaderStageExtension;

	switch (shaderStage) {
	case ShaderStage::Vertex:
		shaderStageExtension = ".vert";
		break;
	case ShaderStage::Fragment:
		shaderStageExtension = ".frag";
		break;
	case ShaderStage::TesselationEvaluation:
		shaderStageExtension = ".eval";
		break;
	case ShaderStage::TesselationControl:
		shaderStageExtension = ".ctrl";
		break;
	case ShaderStage::Geometry:
		shaderStageExtension = ".geom";
		break;
	case ShaderStage::Compute:
		shaderStageExtension = ".comp";
		break;
	default:
		std::cout << "Incorrect shader stage" << std::endl;
		break;
	}

	return std::string(basePath) + shaderStageExtension + core->GetDefaultShaderExtension();
}

Shader& ShaderManager::LoadShader(const char* path) {
	Shader* shader = nullptr;
	if (TryGetShader(path, shader)) {
		return *shader;
	}

	return CreateShaderFromFile(path);
}

bool ShaderManager::TryGetShader(const char* path, Shader*& shader) {
	auto& foundShader = shaders.find(path);
	if (foundShader != shaders.end()) {
		shader = &foundShader->second;
		return true;
	}

	return false;
}

Shader& ShaderManager::CreateShaderFromFile(const char* path) {
	shaders[path] = Shader{ path };
	auto& shader = shaders[path];
	CreateReflectionDataForShader(path, shader);
	CreateShaderGraphicsPipeline(path, shader);

	return shader;
}

void ShaderManager::CreateReflectionDataForShader(const char* path, Shader& shader) {
	LoadShaderReflection(path, shader.reflectionData);
}

void ShaderManager::CreateShaderGraphicsPipeline(const char* basePath, Shader& shader) {
	GraphicsAPI::Core* core;

	auto& shaderStagesBitMask = shader.reflectionData.shaderStagesBitMask;
	size_t numShaderStages = shader.reflectionData.numShaderStages;
	std::vector<ShaderStageCreateInfo> shaderStages;
	shaderStages.resize(numShaderStages);
	size_t currentShaderStage = 0;

	for(
		ShaderStage stage = ShaderStage::Vertex;
		stage < ShaderStage::Compute;
		stage=(ShaderStage)((uint8_t)stage+1)
	) {
		uint8_t stageBit = (1 << (uint8_t)stage);
		if ((stageBit & shaderStagesBitMask) != stageBit) {
			return;
		}

		auto& stageCreateInfo = shaderStages[currentShaderStage++];
		std::string path = GetShaderPath(basePath, stage, core);
		stageCreateInfo.fileName = path.c_str();

		if (!std::filesystem::exists(path)) {
			return;
		}

		auto file = Utils::LoadFile(path.c_str());
		stageCreateInfo.content = file.data();
		stageCreateInfo.size = file.size();
		stageCreateInfo.type = stage;
	}

	Pipeline::CreateInfo pipelineCi{};
	pipelineCi.primitiveType = GeometryType::Triangles;
	pipelineCi.cullMode = CullMode::None;
	pipelineCi.renderPass;
	pipelineCi.width = 800;
	pipelineCi.height = 600;
	pipelineCi.scissorX = 0;
	pipelineCi.scissorY = 0;
	pipelineCi.scissorW = 800;
	pipelineCi.scissorH = 600;
	pipelineCi.shaderStageCreateInfos = shaderStages.data();
	pipelineCi.shaderStageCreateInfoCount = shaderStages.size();

	pipelineCi.uniformBufferBindings = nullptr;
	pipelineCi.uniformBufferBindingCount = 0;

	pipelineCi.textureBindings = nullptr;
	pipelineCi.textureBindingCount = 0;

	pipelineCi.vertexBindings = nullptr;
	pipelineCi.vertexBindingsCount = 0;
	shader.pipeline = core->CreatePipeline(pipelineCi);
}
