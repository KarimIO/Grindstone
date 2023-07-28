#include <filesystem>
#include "ShaderImporter.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Common/Graphics/Core.hpp"
#include "ShaderReflectionLoader.hpp"
#include "EngineCore/Rendering/DeferredRenderer.hpp"
#include "EngineCore/Logger.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "EngineCore/AssetRenderer/AssetRendererManager.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

// TODO : Remove ShaderLayoutIndex and ShaderVertexLayouts, insert them via plugins, and search via dictionary
enum class ShaderLayoutIndex {
	Position = 0,
	Normal,
	Tangent,
	Uv0
};

struct ShaderVertexLayouts {
	GraphicsAPI::VertexBufferLayout positions;
	GraphicsAPI::VertexBufferLayout normals;
	GraphicsAPI::VertexBufferLayout tangents;
	GraphicsAPI::VertexBufferLayout uv0;
};

Grindstone::GraphicsAPI::CullMode TranslateCullMode(std::string& cullMode) {
	if (cullMode == "Front") {
		return CullMode::Front;
	}
	if (cullMode == "None") {
		return CullMode::None;
	}
	if (cullMode == "Both") {
		return CullMode::Both;
	}

	return CullMode::Back;
}

void* ShaderImporter::ProcessLoadedFile(Uuid uuid) {
	auto asset = shaders.emplace(uuid, ShaderAsset(uuid));
	ShaderAsset& shaderAsset = asset.first->second;

	if (!ImportShader(shaderAsset)) {
		return nullptr;
	}

	return &shaderAsset;
}

bool ShaderImporter::ImportShader(ShaderAsset& shaderAsset) {
	// TODO: Check shader cache before loading and compiling again
	// The shader cache includes shaders precompiled for consoles, or compiled once per driver update on computers
	// if shaderCache has shader with this uuid
	//		return shader

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;
	AssetRendererManager* assetRendererManager = EngineCore::GetInstance().assetRendererManager;

	std::string outContent;
	if (!assetManager->LoadFileText(shaderAsset.uuid, outContent)) {
		std::string errorMsg = shaderAsset.uuid.ToString() + " shader reflection file not found.";
		EngineCore::GetInstance().Print(LogSeverity::Error, errorMsg.c_str());
		return false;
	}

	ShaderReflectionData reflectionData;
	ShaderReflectionLoader loader(outContent.data(), reflectionData);
	std::vector<ShaderStageCreateInfo> shaderStageCreateInfos;
	std::vector<std::vector<char>> fileData;

	if (!assetManager->LoadShaderSet(
		shaderAsset.uuid,
		reflectionData.shaderStagesBitMask,
		reflectionData.numShaderStages,
		shaderStageCreateInfos,
		fileData
	)) {
		return false;
	}

	std::string debugName = reflectionData.name;

	bool usesGbuffer = reflectionData.renderQueue != "Skybox";

	auto renderPass = usesGbuffer
		? DeferredRenderer::gbufferRenderPass
		: DeferredRenderer::mainRenderPass;
	GraphicsPipeline::CreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.debugName = debugName.c_str();
	pipelineCreateInfo.primitiveType = GeometryType::Triangles;
	pipelineCreateInfo.cullMode = TranslateCullMode(reflectionData.cullMode);
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.width = static_cast<float>(renderPass->GetWidth());
	pipelineCreateInfo.height = static_cast<float>(renderPass->GetHeight());
	pipelineCreateInfo.scissorX = 0;
	pipelineCreateInfo.scissorY = 0;
	pipelineCreateInfo.scissorW = renderPass->GetWidth();
	pipelineCreateInfo.scissorH = renderPass->GetHeight();
	pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
	pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
	pipelineCreateInfo.hasDynamicViewport = true;
	pipelineCreateInfo.hasDynamicScissor = true;

	const size_t descriptorSetCount = 3;
	std::array<std::vector<GraphicsAPI::DescriptorSetLayout::Binding>, descriptorSetCount> descriptorSetBindings;

	for (auto& uniform : reflectionData.uniformBuffers) {
		uint32_t set = uniform.setId;
		if (set >= descriptorSetCount) {
			continue;
		}

		GraphicsAPI::DescriptorSetLayout::Binding layoutBindingCi{};
		layoutBindingCi.bindingId = static_cast<uint32_t>(uniform.bindingId);
		layoutBindingCi.type = BindingType::UniformBuffer;
		layoutBindingCi.stages = static_cast<GraphicsAPI::ShaderStageBit>(uniform.shaderStagesBitMask);
		layoutBindingCi.count = 1;
		descriptorSetBindings[set].emplace_back(layoutBindingCi);
	}

	for (auto& textureBinding : reflectionData.textures) {
		uint32_t set = textureBinding.setId;
		if (set >= descriptorSetCount) {
			continue;
		}

		GraphicsAPI::DescriptorSetLayout::Binding layoutBindingCi{};
		layoutBindingCi.bindingId = static_cast<uint32_t>(textureBinding.bindingId);
		layoutBindingCi.type = BindingType::Texture;
		layoutBindingCi.count = 1;
		layoutBindingCi.stages = static_cast<GraphicsAPI::ShaderStageBit>(textureBinding.shaderStagesBitMask);
		descriptorSetBindings[set].emplace_back(layoutBindingCi);
	}

	std::array<GraphicsAPI::DescriptorSetLayout*, descriptorSetCount> descriptorSetLayouts{};
	for (size_t i = 0; i < descriptorSetCount; ++i) {
		auto& bindings = descriptorSetBindings[i];
		GraphicsAPI::DescriptorSetLayout::CreateInfo layoutCi{};
		layoutCi.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutCi.bindings = bindings.data();
		descriptorSetLayouts[i] = graphicsCore->CreateDescriptorSetLayout(layoutCi);
	}

	pipelineCreateInfo.descriptorSetLayouts = descriptorSetLayouts.data();
	pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());

	ShaderVertexLayouts vertexLayouts;
	vertexLayouts.positions = {
		{
			(uint32_t)ShaderLayoutIndex::Position,
			Grindstone::GraphicsAPI::VertexFormat::Float3,
			"vertexPosition",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::Position
		}
	};

	vertexLayouts.normals = {
		{
			(uint32_t)ShaderLayoutIndex::Normal,
			Grindstone::GraphicsAPI::VertexFormat::Float3,
			"vertexNormal",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::Normal
		}
	};

	vertexLayouts.tangents = {
		{
			(uint32_t)ShaderLayoutIndex::Tangent,
			Grindstone::GraphicsAPI::VertexFormat::Float3,
			"vertexTangent",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::Tangent
		}
	};

	vertexLayouts.uv0 = {
		{
			(uint32_t)ShaderLayoutIndex::Uv0,
			Grindstone::GraphicsAPI::VertexFormat::Float2,
			"vertexTexCoord0",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::TexCoord0
		}
	};

	pipelineCreateInfo.vertexBindings = &vertexLayouts.positions;
	pipelineCreateInfo.vertexBindingsCount = 4; // Would be 5, but uv1 is not yet used
	pipelineCreateInfo.colorAttachmentCount = usesGbuffer ? 4 : 1; // TODO: Collor Attachments depending on stage and renderer type
	pipelineCreateInfo.blendMode = BlendMode::None; // TODO: Support Blending
	pipelineCreateInfo.isDepthTestEnabled = true;
	pipelineCreateInfo.isDepthWriteEnabled = true;
	pipelineCreateInfo.isStencilEnabled = false;

	GraphicsPipeline* pipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
	shaderAsset.reflectionData = reflectionData;
	shaderAsset.descriptorSetLayout = pipelineCreateInfo.descriptorSetLayouts[0];
	shaderAsset.pipeline = pipeline;

	std::string& renderQueue = shaderAsset.reflectionData.renderQueue;
	std::string& geometryType = shaderAsset.reflectionData.geometryRenderer;
	assetRendererManager->RegisterShader(geometryType, renderQueue, shaderAsset.uuid);

	// TODO: Save compiled shader into ShaderCache

	return true;
}

bool ShaderImporter::TryGetIfLoaded(Uuid uuid, void*& output) {
	auto& shaderInMap = shaders.find(uuid);
	if (shaderInMap != shaders.end()) {
		output = &shaderInMap->second;
		return true;
	}

	return false;
}

void ShaderImporter::QueueReloadAsset(Uuid uuid) {
	auto& shaderInMap = shaders.find(uuid);
	if (shaderInMap == shaders.end()) {
		return;
	}

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	GraphicsPipeline*& pipeline = shaderInMap->second.pipeline;
	if (pipeline != nullptr) {
		graphicsCore->DeleteGraphicsPipeline(pipeline);
		pipeline = nullptr;
	}

	ImportShader(shaderInMap->second);
}
