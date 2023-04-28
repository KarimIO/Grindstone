#include <filesystem>
#include "ShaderImporter.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Common/Graphics/Core.hpp"
#include "ShaderReflectionLoader.hpp"
#include "EngineCore/Rendering/DeferredRenderer.hpp"
#include "EngineCore/Logger.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
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

void* ShaderImporter::ProcessLoadedFile(Uuid uuid) {
	// TODO: Check shader cache before loading and compiling again
	// The shader cache includes shaders precompiled for consoles, or compiled once per driver update on computers
	// if shaderCache has shader with this uuid
	//		return shader

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;

	std::string outContent;
	if (!assetManager->LoadFileText(uuid, outContent)) {
		std::string errorMsg = uuid.ToString() + " shader reflection file not found.";
		EngineCore::GetInstance().Print(LogSeverity::Error, errorMsg.c_str());
		return nullptr;
	}

	ShaderReflectionData reflectionData;
	ShaderReflectionLoader loader(outContent.data(), reflectionData);
	auto& shaderStagesBitMask = reflectionData.shaderStagesBitMask;
	size_t numShaderStages = reflectionData.numShaderStages;
	std::vector<ShaderStageCreateInfo> shaderStages;
	shaderStages.resize(numShaderStages);
	size_t currentShaderStage = 0;

	std::vector<std::string> fileNames;
	fileNames.resize(numShaderStages);
	std::vector<std::vector<char>> fileData;
	fileData.resize(numShaderStages);
	size_t fileDataIterator = 0;

	std::string basePath = EngineCore::GetInstance().GetAssetsPath().string() + uuid.ToString();

	for (
		ShaderStage stage = ShaderStage::Vertex;
		stage < ShaderStage::Compute;
		stage = (ShaderStage)((uint8_t)stage + 1)
	) {
		uint8_t stageBit = (1 << (uint8_t)stage);
		if ((stageBit & shaderStagesBitMask) != stageBit) {
			continue;
		}

		auto& stageCreateInfo = shaderStages[currentShaderStage++];
		auto& path = fileNames[fileDataIterator] = GetShaderPath(basePath.c_str(), stage, graphicsCore);
		stageCreateInfo.fileName = path.c_str();

		if (!std::filesystem::exists(path)) {
			std::string errorMsg = path + " shader not found.";
			EngineCore::GetInstance().Print(LogSeverity::Error, errorMsg.c_str());
			return nullptr;
		}

		fileData[fileDataIterator] = Utils::LoadFile(path.c_str());
		auto& file = fileData[fileDataIterator];
		stageCreateInfo.content = file.data();
		stageCreateInfo.size = (uint32_t)file.size();
		stageCreateInfo.type = stage;

		++fileDataIterator;
	}

	std::string debugName = reflectionData.name;

	Pipeline::CreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.shaderName = debugName.c_str();
	pipelineCreateInfo.primitiveType = GeometryType::Triangles;
	pipelineCreateInfo.cullMode = CullMode::None;
	pipelineCreateInfo.renderPass = DeferredRenderer::gbufferRenderPass;
	pipelineCreateInfo.width = 800;
	pipelineCreateInfo.height = 600;
	pipelineCreateInfo.scissorX = 0;
	pipelineCreateInfo.scissorY = 0;
	pipelineCreateInfo.scissorW = 800;
	pipelineCreateInfo.scissorH = 600;
	pipelineCreateInfo.shaderStageCreateInfos = shaderStages.data();
	pipelineCreateInfo.shaderStageCreateInfoCount = (uint32_t)shaderStages.size();

	std::vector<GraphicsAPI::UniformBufferBinding*> ubbs;
	ubbs.reserve(reflectionData.uniformBuffers.size());

	for (auto& uniform : reflectionData.uniformBuffers) {
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

	/*
	GraphicsAPI::TextureBindingLayout::CreateInfo textureBindingLayoutCreateInfo{};
	textureBindingLayoutCreateInfo.bindingLocation = 2;
	textureBindingLayoutCreateInfo.bindingCount = 1;
	textureBindingLayoutCreateInfo.bindings = &sub;
	textureBindingLayoutCreateInfo.stages = GraphicsAPI::ShaderStageBit::All;
	auto textureBindingLayout = graphicsCore->CreateTextureBindingLayout(textureBindingLayoutCreateInfo);
	*/

	pipelineCreateInfo.textureBindings = nullptr; //textureBindingLayout;
	pipelineCreateInfo.textureBindingCount = 0;

	pipelineCreateInfo.vertexBindings = nullptr;
	pipelineCreateInfo.vertexBindingsCount = 0;

	auto shader = graphicsCore->CreatePipeline(pipelineCreateInfo);
	auto asset = shaders.emplace(uuid, ShaderAsset(uuid, debugName, shader));
	auto& shaderAsset = asset.first->second;
	shaderAsset.reflectionData = reflectionData;
	shaderAsset.textureBindingLayout = nullptr;

	// TODO: Save compiled shader into ShaderCache

	return &shaderAsset;
}

bool ShaderImporter::TryGetIfLoaded(Uuid uuid, void*& output) {
	auto& shaderInMap = shaders.find(uuid);
	if (shaderInMap != shaders.end()) {
		output = &shaderInMap->second;
		return true;
	}

	return false;
}

#if 0
void ShaderImporter::RemoveMaterialFromShader(Shader* shader, Material *material) {
	for (size_t i = 0; i < shader->materials.size(); ++i) {
		if (shader->materials[i] == material) {
			shader->materials.erase(shader->materials.begin() + i);
		}
	}
}

void ShaderImporter::LoadShaderFromFile(bool isReloading, std::string& path, Shader& shaderAsset) {
	CreateReflectionDataForShader(path.c_str(), shaderAsset);
	CreateShaderGraphicsPipeline(isReloading, path.c_str(), shaderAsset);
}

Shader& ShaderImporter::CreateNewShaderFromFile(std::string& uuid) {
	std::string path = EngineCore::GetInstance().GetAssetPath(uuid).string();
	Shader& shader = shaders[uuid] = Shader();
	shader.uuid = uuid;
	LoadShaderFromFile(false, path, shader);

	return shader;
}

void ShaderImporter::CreateReflectionDataForShader(const char* path, Shader& shader) {
	LoadShaderReflection(path, shader.reflectionData);
}
#endif
