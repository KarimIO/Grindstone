#include <filesystem>

#include <Common/Formats/PipelineSet.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>

#include "ComputePipelineImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Containers;
using namespace Grindstone::GraphicsAPI;
using namespace Grindstone::Formats::Pipelines;

static void UnpackComputePipelineDescriptorSetHeaders(
	Grindstone::GraphicsAPI::Core* graphicsCore,
	std::string_view pipelineName,
	const Span<V1::ShaderReflectDescriptorSet>& srcDescriptorSets,
	const Span<V1::ShaderReflectDescriptorBinding>& srcDescriptorBindings,
	std::array<Grindstone::GraphicsAPI::DescriptorSetLayout*, 16>& dstDescriptorSets
) {
	Grindstone::GraphicsAPI::DescriptorSetLayout::CreateInfo layoutCreateInfo;

	for (uint8_t i = 0; i < srcDescriptorSets.GetSize(); ++i) {
		const V1::ShaderReflectDescriptorSet& srcDescriptorSet = srcDescriptorSets[i];

		std::vector<Grindstone::GraphicsAPI::DescriptorSetLayout::Binding> dstDescriptorBindings;
		dstDescriptorBindings.reserve(srcDescriptorSet.bindingCount);

		for (size_t bindingIndex = srcDescriptorSet.bindingStartIndex;
			bindingIndex < srcDescriptorSet.bindingStartIndex + srcDescriptorSet.bindingCount;
			++bindingIndex
		) {
			const V1::ShaderReflectDescriptorBinding& srcBinding = srcDescriptorBindings[bindingIndex];
			Grindstone::GraphicsAPI::DescriptorSetLayout::Binding& dstBinding = dstDescriptorBindings.emplace_back();
			dstBinding.bindingId = srcBinding.bindingIndex;
			dstBinding.stages = srcBinding.stages;
			dstBinding.count = srcBinding.count;
			dstBinding.type = srcBinding.type;
		}

		std::string setName = std::vformat("{} Descriptor Set {}", std::make_format_args(pipelineName, i));
		layoutCreateInfo.debugName = setName.c_str();
		layoutCreateInfo.bindings = dstDescriptorBindings.data();
		layoutCreateInfo.bindingCount = static_cast<uint32_t>(dstDescriptorBindings.size());

		dstDescriptorSets[srcDescriptorSet.setIndex] = graphicsCore->CreateDescriptorSetLayout(layoutCreateInfo);
	}
}

static bool ImportComputeAsset(ComputePipelineAsset& computePipelineAsset) {
	// TODO: Check shader cache before loading and compiling again
	// The shader cache includes shaders precompiled for consoles, or compiled once per driver update on computers
	// if shaderCache has shader with this uuid
	//		return shader

	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Assets::AssetManager* assetManager = engineCore.assetManager;
	AssetRendererManager* assetRendererManager = engineCore.assetRendererManager;

	Assets::AssetLoadBinaryResult result = assetManager->LoadBinaryByUuid(AssetType::ComputePipelineSet, computePipelineAsset.uuid);
	if (result.status != Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not find shader with id {}.", computePipelineAsset.uuid.ToString());
		return false;
	}

	Grindstone::Buffer& fileData = result.buffer;
	GS_ASSERT(fileData.GetCapacity() >= (4 + sizeof(V1::PipelineSetFileHeader)));

	if (memcmp(fileData.Get(), V1::FileMagicCode, 4) != 0) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Graphics Pipeline file does not start with GPSF - {}.", result.displayName);
		computePipelineAsset.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	computePipelineAsset.name = result.displayName;

	unsigned char* readPtr = fileData.Get() + 4;
	V1::PipelineSetFileHeader* srcFileHeader = reinterpret_cast<V1::PipelineSetFileHeader*>(readPtr);
	readPtr += sizeof(V1::PipelineSetFileHeader);

	GS_ASSERT(srcFileHeader->headerSize == sizeof(V1::PipelineSetFileHeader));
	GS_ASSERT(srcFileHeader->graphicsPipelineSize == sizeof(V1::GraphicsPipelineSetHeader));
	GS_ASSERT(srcFileHeader->computePipelineSize == sizeof(V1::ComputePipelineSetHeader));
	GS_ASSERT(srcFileHeader->graphicsConfigurationSize == sizeof(V1::GraphicsPipelineConfigurationHeader));
	GS_ASSERT(srcFileHeader->computeConfigurationSize == sizeof(V1::ComputePipelineConfigurationHeader));
	GS_ASSERT(srcFileHeader->passSize == sizeof(V1::PassPipelineHeader));
	GS_ASSERT(srcFileHeader->attachmentSize == sizeof(V1::PassPipelineAttachmentHeader));
	GS_ASSERT(srcFileHeader->stageSize == sizeof(V1::PassPipelineShaderStageHeader));

	Span<V1::ComputePipelineSetHeader> computePipelines = fileData.GetSpan<V1::ComputePipelineSetHeader>(srcFileHeader->computePipelinesOffset, srcFileHeader->computePipelineCount);
	Span<V1::ComputePipelineConfigurationHeader> pipelineConfigurations = fileData.GetSpan<V1::ComputePipelineConfigurationHeader>(srcFileHeader->computeConfigurationsOffset, srcFileHeader->computeConfigurationCount);
	Span<V1::PassPipelineShaderStageHeader> shaderStages = fileData.GetSpan<V1::PassPipelineShaderStageHeader>(srcFileHeader->shaderStagesOffset, srcFileHeader->shaderStageCount);
	Span<V1::ShaderReflectDescriptorSet> descriptorSets = fileData.GetSpan<V1::ShaderReflectDescriptorSet>(srcFileHeader->descriptorSetsOffset, srcFileHeader->descriptorSetCount);
	Span<V1::ShaderReflectDescriptorBinding> descriptorBindings = fileData.GetSpan<V1::ShaderReflectDescriptorBinding>(srcFileHeader->descriptorBindingsOffset, srcFileHeader->descriptorBindingCount);
	BufferSpan blobs = fileData.GetSpan(srcFileHeader->blobSectionOffset, srcFileHeader->blobSectionSize);

	GS_ASSERT(computePipelines.GetSize() != 0);
	const V1::ComputePipelineSetHeader& srcPipelineHeader = computePipelines[0];

	GS_ASSERT(srcPipelineHeader.configurationCount != 0);
	const V1::ComputePipelineConfigurationHeader& srcConfigHeader = pipelineConfigurations[srcPipelineHeader.configurationStartIndex];
	const V1::PassPipelineShaderStageHeader& srcStage = shaderStages[srcConfigHeader.shaderStageIndex];

	UnpackComputePipelineDescriptorSetHeaders(
		graphicsCore,
		computePipelineAsset.name,
		descriptorSets.GetSubspan(srcConfigHeader.descriptorSetStartIndex, srcConfigHeader.descriptorSetCount),
		descriptorBindings,
		computePipelineAsset.descriptorSetLayouts
	);


	// Get the highest descriptor set index
	uint32_t descriptorSetLayoutCount = 0;
	for (uint8_t descriptorSetIndex = 0; descriptorSetIndex < srcConfigHeader.descriptorSetCount; ++descriptorSetIndex) {
		auto& descriptorSet = descriptorSets[descriptorSetIndex];
		uint32_t indexAsCount = descriptorSet.setIndex + 1;
		descriptorSetLayoutCount = descriptorSetLayoutCount > indexAsCount
			? descriptorSetLayoutCount
			: indexAsCount;
	}

	for (uint8_t descriptorSetIndex = 0; descriptorSetIndex < descriptorSetLayoutCount; ++descriptorSetIndex) {
		GraphicsAPI::DescriptorSetLayout*& descriptorSetLayout = computePipelineAsset.descriptorSetLayouts[descriptorSetIndex];
		if (descriptorSetLayout == nullptr) {
			GraphicsAPI::DescriptorSetLayout::CreateInfo ci{};
			ci.debugName = "Unbound descriptor set";
			ci.bindingCount = 0;
			ci.bindings = nullptr;
			descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ci);
		}
	}

	Grindstone::GraphicsAPI::ComputePipeline::CreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.debugName = computePipelineAsset.name.c_str();
	pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(descriptorSetLayoutCount);
	pipelineCreateInfo.descriptorSetLayouts = computePipelineAsset.descriptorSetLayouts.data();
	pipelineCreateInfo.shaderContent = reinterpret_cast<const char*>(&blobs.GetBegin() + srcStage.shaderCodeOffsetFromBlobStart);
	pipelineCreateInfo.shaderFileName = computePipelineAsset.name.c_str();
	pipelineCreateInfo.shaderSize = srcStage.shaderCodeSize;
	computePipelineAsset.pipeline = graphicsCore->CreateComputePipeline(pipelineCreateInfo);

	// TODO: Save compiled shader into ShaderCache

	computePipelineAsset.assetLoadStatus = AssetLoadStatus::Ready;
	return true;
}

void* ComputePipelineImporter::LoadAsset(Uuid uuid) {
	auto pipelineIterator = assets.emplace(uuid, ComputePipelineAsset(uuid));
	ComputePipelineAsset& computePipelineAsset = pipelineIterator.first->second;

	computePipelineAsset.assetLoadStatus = AssetLoadStatus::Loading;
	if (!ImportComputeAsset(computePipelineAsset)) {
		return nullptr;
	}

	return &computePipelineAsset;
}

void ComputePipelineImporter::QueueReloadAsset(Uuid uuid) {
	auto shaderInMap = assets.find(uuid);
	if (shaderInMap == assets.end()) {
		return;
	}

	Grindstone::ComputePipelineAsset& computePipelineAsset = shaderInMap->second;

	computePipelineAsset.assetLoadStatus = AssetLoadStatus::Reloading;
	Grindstone::GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	graphicsCore->DeleteComputePipeline(computePipelineAsset.pipeline);

	ImportComputeAsset(computePipelineAsset);
}

ComputePipelineImporter::~ComputePipelineImporter() {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	for (auto& asset : assets) {
		graphicsCore->DeleteComputePipeline(asset.second.pipeline);
	}
	assets.clear();
}
