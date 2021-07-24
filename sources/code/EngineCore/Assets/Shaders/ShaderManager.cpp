#include <filesystem>
#include "ShaderManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Common/Graphics/Core.hpp"
#include "ShaderReflectionLoader.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

std::string GetShaderPath(const char* basePath, ShaderStage shaderStage, GraphicsAPI::Core* graphicsCore) {
	const char* shaderStageExtension = "";

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

	return std::string(basePath) + shaderStageExtension + graphicsCore->GetDefaultShaderExtension();
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
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().getGraphicsCore();

	auto& shaderStagesBitMask = shader.reflectionData.shaderStagesBitMask;
	size_t numShaderStages = shader.reflectionData.numShaderStages;
	std::vector<ShaderStageCreateInfo> shaderStages;
	shaderStages.resize(numShaderStages);
	size_t currentShaderStage = 0;

	std::vector<std::vector<char>> fileData;
	fileData.resize(2);
	size_t fileDataIterator = 0;

	for(
		ShaderStage stage = ShaderStage::Vertex;
		stage < ShaderStage::Compute;
		stage=(ShaderStage)((uint8_t)stage+1)
	) {
		uint8_t stageBit = (1 << (uint8_t)stage);
		if ((stageBit & shaderStagesBitMask) != stageBit) {
			continue;
		}

		auto& stageCreateInfo = shaderStages[currentShaderStage++];
		std::string path = GetShaderPath(basePath, stage, graphicsCore);
		stageCreateInfo.fileName = path.c_str();

		if (!std::filesystem::exists(path)) {
			throw std::runtime_error(std::string(path) + " shader not found.");
		}

		fileData[fileDataIterator] = Utils::LoadFile(path.c_str());
		auto& file = fileData[fileDataIterator];
		stageCreateInfo.content = file.data();
		stageCreateInfo.size = (uint32_t)file.size();
		stageCreateInfo.type = stage;

		++fileDataIterator;
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
	pipelineCi.shaderStageCreateInfoCount = (uint32_t)shaderStages.size();

	std::vector<GraphicsAPI::UniformBufferBinding*> ubbs;
	ubbs.resize(2);

	GraphicsAPI::UniformBufferBinding::CreateInfo ubbCi{};
	ubbCi.binding = 0;
	ubbCi.shaderLocation = "EngineUbo";
	ubbCi.size = 64 * 3;
	ubbCi.stages = GraphicsAPI::ShaderStageBit::All;
	ubbs[0] = graphicsCore->CreateUniformBufferBinding(ubbCi);

	ubbCi.binding = 1;
	ubbCi.shaderLocation = "MaterialUbo";
	ubbCi.size = 16;
	ubbCi.stages = GraphicsAPI::ShaderStageBit::All;
	ubbs[1] = graphicsCore->CreateUniformBufferBinding(ubbCi);


	pipelineCi.uniformBufferBindings = ubbs.data();
	pipelineCi.uniformBufferBindingCount = (uint32_t)ubbs.size();

	pipelineCi.textureBindings = nullptr;
	pipelineCi.textureBindingCount = 0;

	pipelineCi.vertexBindings = nullptr;
	pipelineCi.vertexBindingsCount = 0;
	shader.pipeline = graphicsCore->CreatePipeline(pipelineCi);
}
