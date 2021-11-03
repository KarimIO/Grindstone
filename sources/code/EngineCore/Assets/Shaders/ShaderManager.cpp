#include <filesystem>
#include "ShaderManager.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Common/Graphics/Core.hpp"
#include "ShaderReflectionLoader.hpp"
#include "EngineCore/Assets/BaseAssetRenderer.hpp"
#include "EngineCore/Logger.hpp"
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
		Logger::PrintError("Incorrect shader stage");
		break;
	}

	return std::string(basePath) + shaderStageExtension + graphicsCore->GetDefaultShaderExtension();
}

Shader& ShaderManager::LoadShader(BaseAssetRenderer* assetRenderer, const char* uuid) {
	Shader* shader = nullptr;
	if (!TryGetShader(uuid, shader)) {
		std::string uuidStr = uuid;
		shader = &CreateNewShaderFromFile(uuidStr);

		if (assetRenderer) {
			assetRenderer->AddShaderToRenderQueue(shader);
		}
	}

	return *shader;
}

void ShaderManager::ReloadShaderIfLoaded(const char* path) {
	std::string fixedPath = path;
	Utils::FixStringSlashes(fixedPath);

	Shader* shader = nullptr;
	if (TryGetShader(fixedPath.c_str(), shader)) {
		LoadShaderFromFile(true, fixedPath, *shader);
	}
}

bool ShaderManager::TryGetShader(const char* path, Shader*& shader) {
	auto& foundShader = shaders.find(path);
	if (foundShader != shaders.end()) {
		shader = &foundShader->second;
		return true;
	}

	return false;
}

void ShaderManager::LoadShaderFromFile(bool isReloading, std::string& path, Shader& shaderAsset) {
	CreateReflectionDataForShader(path.c_str(), shaderAsset);
	CreateShaderGraphicsPipeline(isReloading, path.c_str(), shaderAsset);
}

Shader& ShaderManager::CreateNewShaderFromFile(std::string& uuid) {
	std::string path = "../compiledAssets/" + uuid;
	Shader& shader = shaders[uuid] = Shader{ path };
	LoadShaderFromFile(false, path, shader);

	return shader;
}

void ShaderManager::CreateReflectionDataForShader(const char* path, Shader& shader) {
	LoadShaderReflection(path, shader.reflectionData);
}

void ShaderManager::CreateShaderGraphicsPipeline(bool isReloading, const char* basePath, Shader& shader) {
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

	Pipeline::CreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.shaderName = shader.reflectionData.name.c_str();
	pipelineCreateInfo.primitiveType = GeometryType::Triangles;
	pipelineCreateInfo.cullMode = CullMode::None;
	pipelineCreateInfo.renderPass;
	pipelineCreateInfo.width = 800;
	pipelineCreateInfo.height = 600;
	pipelineCreateInfo.scissorX = 0;
	pipelineCreateInfo.scissorY = 0;
	pipelineCreateInfo.scissorW = 800;
	pipelineCreateInfo.scissorH = 600;
	pipelineCreateInfo.shaderStageCreateInfos = shaderStages.data();
	pipelineCreateInfo.shaderStageCreateInfoCount = (uint32_t)shaderStages.size();

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

	pipelineCreateInfo.uniformBufferBindings = ubbs.data();
	pipelineCreateInfo.uniformBufferBindingCount = (uint32_t)ubbs.size();

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

	pipelineCreateInfo.textureBindings = &textureBindingLayout;
	pipelineCreateInfo.textureBindingCount = 1;

	pipelineCreateInfo.vertexBindings = nullptr;
	pipelineCreateInfo.vertexBindingsCount = 0;

	if (isReloading) {
		shader.pipeline->Recreate(pipelineCreateInfo);
	}
	else {
		shader.pipeline = graphicsCore->CreatePipeline(pipelineCreateInfo);
	}
}
