#include <filesystem>
#include "ShaderManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Common/Graphics/Core.hpp"
#include "ShaderReflectionLoader.hpp"
#include "EngineCore/Assets/BaseAssetRenderer.hpp"
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

Shader& ShaderManager::LoadShader(BaseAssetRenderer* assetRenderer, const char* path) {
	Shader* shader = nullptr;
	if (!TryGetShader(path, shader)) {
		shader = &CreateShaderFromFile(path);
	}

	if (assetRenderer) {
		assetRenderer->AddShaderToRenderQueue(shader);
	}

	return *shader;
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
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	auto& shaderStagesBitMask = shader.reflectionData.shaderStagesBitMask;
	size_t numShaderStages = shader.reflectionData.numShaderStages;
	std::vector<ShaderStageCreateInfo> shaderStages;
	shaderStages.resize(numShaderStages);
	size_t currentShaderStage = 0;

	std::vector<std::string> fileNames;
	fileNames.resize(numShaderStages);
	std::vector<std::vector<char>> fileData;
	fileData.resize(numShaderStages);
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
		auto& path = fileNames[fileDataIterator] = GetShaderPath(basePath, stage, graphicsCore);
		stageCreateInfo.fileName = path.c_str();

		if (!std::filesystem::exists(path)) {
			throw std::runtime_error(path + " shader not found.");
		}

		fileData[fileDataIterator] = Utils::LoadFile(path.c_str());
		auto& file = fileData[fileDataIterator];
		stageCreateInfo.content = file.data();
		stageCreateInfo.size = (uint32_t)file.size();
		stageCreateInfo.type = stage;

		++fileDataIterator;
	}

	Pipeline::CreateInfo pipelineCi{};
	pipelineCi.shaderName = shader.reflectionData.name.c_str();
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
	ubbs.reserve(shader.reflectionData.uniformBuffers.size());

	for (auto& uniform : shader.reflectionData.uniformBuffers) {
		GraphicsAPI::UniformBufferBinding::CreateInfo ubbCi{};
		ubbCi.binding = (uint32_t)uniform.bindingId;
		ubbCi.shaderLocation = uniform.name.c_str();
		ubbCi.size = (uint32_t)uniform.bufferSize;
		ubbCi.stages = (GraphicsAPI::ShaderStageBit)uniform.shaderStagesBitMask;
		ubbs.push_back(graphicsCore->CreateUniformBufferBinding(ubbCi));
	}

	pipelineCi.uniformBufferBindings = ubbs.data();
	pipelineCi.uniformBufferBindingCount = (uint32_t)ubbs.size();

	GraphicsAPI::TextureSubBinding sub;
	sub.shaderLocation = "texSampler";
	sub.textureLocation = 2;

	GraphicsAPI::TextureBindingLayout::CreateInfo textureBindingLayoutCreateInfo{};
	textureBindingLayoutCreateInfo.bindingLocation = 2;
	textureBindingLayoutCreateInfo.bindingCount = 1;
	textureBindingLayoutCreateInfo.bindings = &sub;
	textureBindingLayoutCreateInfo.stages = GraphicsAPI::ShaderStageBit::All;
	auto textureBindingLayout = graphicsCore->CreateTextureBindingLayout(textureBindingLayoutCreateInfo);
	shader.textureBindingLayout = textureBindingLayout;

	pipelineCi.textureBindings = &textureBindingLayout;
	pipelineCi.textureBindingCount = 1;

	pipelineCi.vertexBindings = nullptr;
	pipelineCi.vertexBindingsCount = 0;
	shader.pipeline = graphicsCore->CreatePipeline(pipelineCi);
}
