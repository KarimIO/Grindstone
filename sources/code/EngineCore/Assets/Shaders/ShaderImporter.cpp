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

	const size_t descriptorSetCount = 3;
	std::array<std::vector<GraphicsAPI::DescriptorSetLayout::Binding>, descriptorSetCount> descriptorSetBindings;

	for (auto& uniform : reflectionData.uniformBuffers) {
		GraphicsAPI::DescriptorSetLayout::Binding layoutBindingCi{};
		layoutBindingCi.bindingId = static_cast<uint32_t>(uniform.bindingId);
		layoutBindingCi.type = BindingType::UniformBuffer;
		layoutBindingCi.stages = static_cast<GraphicsAPI::ShaderStageBit>(uniform.shaderStagesBitMask);
		layoutBindingCi.count = 1;
		uint32_t set = uniform.setId;
		descriptorSetBindings[set].emplace_back(layoutBindingCi);
	}

	for (auto& textureBinding : reflectionData.textures) {
		GraphicsAPI::DescriptorSetLayout::Binding layoutBindingCi{};
		layoutBindingCi.bindingId = static_cast<uint32_t>(textureBinding.bindingId);
		layoutBindingCi.type = BindingType::Texture;
		layoutBindingCi.count = 1;
		layoutBindingCi.stages = static_cast<GraphicsAPI::ShaderStageBit>(textureBinding.shaderStagesBitMask);
		uint32_t set = textureBinding.setId;
		descriptorSetBindings[set].emplace_back(layoutBindingCi);
	}

	std::array<GraphicsAPI::DescriptorSetLayout*, descriptorSetCount> descriptorSetLayouts;
	for (size_t i = 0; i < descriptorSetCount; ++i) {
		auto& bindings = descriptorSetBindings[i];
		GraphicsAPI::DescriptorSetLayout::CreateInfo layoutCi{};
		layoutCi.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutCi.bindings = bindings.data();
		descriptorSetLayouts[i] = graphicsCore->CreateDescriptorSetLayout(layoutCi);
	}

	pipelineCreateInfo.descriptorSetLayouts = descriptorSetLayouts.data();
	pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());

	pipelineCreateInfo.vertexBindings = nullptr;
	pipelineCreateInfo.vertexBindingsCount = 0;

	auto shader = graphicsCore->CreatePipeline(pipelineCreateInfo);
	auto asset = shaders.emplace(uuid, ShaderAsset(uuid, debugName, shader));
	auto& shaderAsset = asset.first->second;
	shaderAsset.reflectionData = reflectionData;
	shaderAsset.descriptorSetLayout = pipelineCreateInfo.descriptorSetLayouts[1];

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
