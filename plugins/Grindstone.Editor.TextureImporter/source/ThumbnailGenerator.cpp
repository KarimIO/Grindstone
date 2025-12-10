#include <Common/Window/WindowManager.hpp>
#include <EngineCore/WorldContext/WorldContextManager.hpp>
#include <EngineCore/CoreComponents/Camera/CameraComponent.hpp>
#include <EngineCore/Rendering/RenderPassRegistry.hpp>
#include <EngineCore/Assets/Textures/TextureAsset.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <Grindstone.Editor.TextureImporter/include/ThumbnailGenerator.hpp>

static struct ThumbnailGeneratorContext {
	Grindstone::GraphicsAPI::RenderPass* renderpass = nullptr;
	Grindstone::GraphicsAPI::Framebuffer* framebuffer = nullptr;
	Grindstone::GraphicsAPI::Image* renderTargetImage = nullptr;
	Grindstone::GraphicsAPI::Sampler* sampler = nullptr;
	Grindstone::GraphicsAPI::CommandBuffer* commandBuffer = nullptr;
	Grindstone::GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
	Grindstone::GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;
	Grindstone::GraphicsAPI::VertexInputLayout vertexInputLayout;
	Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> tex2dGraphicsPipelineAsset;
	Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> cubemapGraphicsPipelineAsset;
} thumbnailGeneratorContext;

#include <Common/Formats/Dds.hpp>
#include "stb_dxt.h"
#include "stb_image_write.h"

static void ExtractBlock(
	const uint8_t* inPtr,
	const uint32_t levelWidth,
	uint8_t* colorBlock
) {
	for (uint32_t dstRow = 0; dstRow < 4; dstRow++) {
		for (uint32_t dstCol = 0; dstCol < 4; dstCol++) {
			memcpy(colorBlock + (dstRow * 4 + dstCol) * 4, inPtr + (dstRow * levelWidth + dstCol) * 4, 4);
		}
	}
}

static void ExportThumbnail(Grindstone::Uuid uuid, void* data) {
	using namespace Grindstone::Formats::DDS;

	DDSHeader outHeader;
	std::memset(&outHeader, 0, sizeof(outHeader));
	outHeader.dwSize = 124;
	outHeader.ddspf.dwFlags = DDPF_FOURCC;
	outHeader.dwFlags = DDSD_REQUIRED;
	outHeader.dwHeight = 128u;
	outHeader.dwWidth = 128u;
	outHeader.dwDepth = 1;
	outHeader.dwCaps = DDSCAPS_TEXTURE;
	outHeader.dwMipMapCount = 1;

	outHeader.dwPitchOrLinearSize = 128u * 128u;
	outHeader.ddspf.dwFourCC = FOURCC_BC3_UNORM;

	char mark[] = { 'G', 'R', 'I', 'N', 'D', 'S', 'T', 'O', 'N', 'E' };
	std::memcpy(&outHeader.dwReserved1, mark, sizeof(mark));

	std::filesystem::path parentPath = Grindstone::EngineCore::GetInstance().GetProjectPath() / "thumbnailCache";
	std::filesystem::create_directories(parentPath);

	std::filesystem::path outputPath = parentPath / uuid.ToString();
	
	std::ofstream out(outputPath, std::ios::binary);
	if (out.fail()) {
		throw std::runtime_error("Failed to output texture!");
	}
	
	std::vector<uint8_t> inputBlockSize;
	inputBlockSize.resize(64);

	uint32_t blockSize = 16u;
	size_t dstOffset = 0u;
	uint8_t* outData = new uint8_t[outHeader.dwPitchOrLinearSize];

	for (uint32_t mipRow = 0; mipRow < 128; mipRow += 4) {
		for (uint32_t mipCol = 0; mipCol < 128; mipCol += 4) {
			uint8_t* ptr = static_cast<uint8_t*>(data) + ((mipRow * 128 + mipCol) * 4);
			ExtractBlock(ptr, 128, inputBlockSize.data());
			stb_compress_dxt_block(&outData[dstOffset], inputBlockSize.data(), true, STB_DXT_NORMAL);
			dstOffset += blockSize;
		}
	}

	const char filecode[4] = { 'D', 'D', 'S', ' ' };
	out.write((const char*)&filecode, sizeof(char) * 4);
	out.write((const char*)&outHeader, sizeof(outHeader));
	out.write((const char*)outData, outHeader.dwPitchOrLinearSize);
	out.close();
}

bool Grindstone::Editor::Importers::GenerateTextureThumbnail(Grindstone::Uuid uuid) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	Grindstone::AssetReference<Grindstone::TextureAsset> texture = engineCore.assetManager->GetAssetReferenceByUuid<Grindstone::TextureAsset>(uuid);

	GraphicsAPI::Image* image = texture.Get()->image;
	if (image->GetImageDimension() != GraphicsAPI::ImageDimension::Dimension2D) {
		return false;
	}

	Grindstone::GraphicsPipelineAsset* pipelineAsset = image->IsCubemap()
		? thumbnailGeneratorContext.cubemapGraphicsPipelineAsset.Get()
		: thumbnailGeneratorContext.tex2dGraphicsPipelineAsset.Get();
	if (pipelineAsset == nullptr) {
		return false;
	}

	Grindstone::GraphicsAPI::GraphicsPipeline* pipeline = pipelineAsset->GetFirstPassPipeline(&thumbnailGeneratorContext.vertexInputLayout);

	Grindstone::GraphicsAPI::RenderPass* renderpass = thumbnailGeneratorContext.renderpass;
	Grindstone::GraphicsAPI::Framebuffer* framebuffer = thumbnailGeneratorContext.framebuffer;

	uint32_t width = 128u;
	uint32_t height = 128u;
	Grindstone::GraphicsAPI::ClearColor clearValues = { 0.0f, 0.0f, 0.0f, 1.0f };
	uint32_t clearCount = 1;
	Grindstone::GraphicsAPI::ClearDepthStencil depthStencilVaue = Grindstone::GraphicsAPI::ClearDepthStencil{};

	Grindstone::GraphicsAPI::DescriptorSet::Binding textureBinding = Grindstone::GraphicsAPI::DescriptorSet::Binding::SampledImage(texture.Get()->image);

	thumbnailGeneratorContext.commandBuffer->BeginCommandBuffer();
	thumbnailGeneratorContext.descriptorSet->ChangeBindings(&textureBinding, 1, 1);
	thumbnailGeneratorContext.commandBuffer->SetViewport(0, 0, 128, 128);
	thumbnailGeneratorContext.commandBuffer->SetScissor(0, 0, 128, 128);
	thumbnailGeneratorContext.commandBuffer->BindRenderPass(renderpass, framebuffer, Math::IntRect2D( 0, 0, width, height ), &clearValues, clearCount, depthStencilVaue);
	thumbnailGeneratorContext.commandBuffer->BindGraphicsPipeline(pipeline);
	thumbnailGeneratorContext.commandBuffer->BindGraphicsDescriptorSet(pipeline, &thumbnailGeneratorContext.descriptorSet, 0, 1);
	thumbnailGeneratorContext.commandBuffer->DrawVertices(6, 0, 1, 0);
	thumbnailGeneratorContext.commandBuffer->UnbindRenderPass();
	thumbnailGeneratorContext.commandBuffer->EndCommandBuffer();

	GraphicsAPI::WindowGraphicsBinding* wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	wgb->SubmitCommandBufferNoSynchronization(thumbnailGeneratorContext.commandBuffer);
	graphicsCore->WaitUntilIdle();

	Grindstone::Buffer data = thumbnailGeneratorContext.renderTargetImage->ReadbackMemory();
	if (data) {
		ExportThumbnail(uuid, data.Get());
	}

	return true;
}

bool Grindstone::Editor::Importers::InitializeTextureThumbnailGenerator() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	Grindstone::GraphicsAPI::Format format = Grindstone::GraphicsAPI::Format::R8G8B8A8_UNORM;

	Grindstone::GraphicsAPI::RenderPass::AttachmentInfo attachmentInfo{
		.colorFormat = format,
		.shouldClear = true,
		.memoryUsage = Grindstone::GraphicsAPI::MemoryUsage::GPUToCPU
	};

	Grindstone::GraphicsAPI::RenderPass::CreateInfo renderpassCreateInfo{
		.debugName = "Thumbnail Renderpass",
		.colorAttachments = &attachmentInfo,
		.colorAttachmentCount = 1u,
		.depthFormat = GraphicsAPI::Format::Invalid,
		.shouldClearDepthOnLoad = false,
	};
	thumbnailGeneratorContext.renderpass = graphicsCore->CreateRenderPass(renderpassCreateInfo);

	Grindstone::HashedString rpName = Grindstone::HashedString("ThumbnailGenerator");
	engineCore.GetRenderPassRegistry()->RegisterRenderpass(rpName, thumbnailGeneratorContext.renderpass);

	Grindstone::GraphicsAPI::Image::CreateInfo renderTargetImageCreateInfo{
		.debugName = "Thumbnail Rendertarget",
		.width = 128u,
		.height = 128u,
		.depth = 1,
		.mipLevels = 1,
		.arrayLayers = 1,
		.format = format,
		.imageDimensions = GraphicsAPI::ImageDimension::Dimension2D,
		.memoryUsage = GraphicsAPI::MemoryUsage::GPUToCPU,
		.imageUsage = GraphicsAPI::ImageUsageFlags::RenderTarget | GraphicsAPI::ImageUsageFlags::TransferSrc,
	};
	thumbnailGeneratorContext.renderTargetImage = graphicsCore->CreateImage(renderTargetImageCreateInfo);

	Grindstone::GraphicsAPI::Framebuffer::CreateInfo framebufferCreateInfo{
		.debugName = "Thumbnail Framebuffer",
		.renderPass = thumbnailGeneratorContext.renderpass,
		.width = 128u,
		.height = 128u,
		.renderTargets = &thumbnailGeneratorContext.renderTargetImage,
		.renderTargetCount = 1u,
		.depthTarget = nullptr,
		.isCubemap = false
	};
	thumbnailGeneratorContext.framebuffer = graphicsCore->CreateFramebuffer(framebufferCreateInfo);

	std::string_view texture2DShaderAddress = "@GRINDSTONE.EDITOR.TEXTUREIMPORTER/editorTextureThumbnailGeneratorShader";
	thumbnailGeneratorContext.tex2dGraphicsPipelineAsset = engineCore.assetManager->GetAssetReferenceByAddress<Grindstone::GraphicsPipelineAsset>(texture2DShaderAddress);

	std::string_view cubemapShaderAddress = "@GRINDSTONE.EDITOR.TEXTUREIMPORTER/editorCubemapThumbnailGeneratorShader";
	thumbnailGeneratorContext.cubemapGraphicsPipelineAsset = engineCore.assetManager->GetAssetReferenceByAddress<Grindstone::GraphicsPipelineAsset>(cubemapShaderAddress);

	thumbnailGeneratorContext.vertexInputLayout.attributes = {};
	thumbnailGeneratorContext.vertexInputLayout.bindings = {};

	Grindstone::GraphicsAPI::CommandBuffer::CreateInfo commandBufferCreateInfo{
		.debugName = "Thumbnail Generator Command Buffer"
	};
	thumbnailGeneratorContext.commandBuffer = graphicsCore->CreateCommandBuffer(commandBufferCreateInfo);

	std::array<Grindstone::GraphicsAPI::DescriptorSetLayout::Binding, 2> layoutBindings = {
		Grindstone::GraphicsAPI::DescriptorSetLayout::Binding{ 0, 1, GraphicsAPI::BindingType::Sampler, GraphicsAPI::ShaderStageBit::Fragment },
		Grindstone::GraphicsAPI::DescriptorSetLayout::Binding{ 1, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }
	};
	Grindstone::GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{
		.debugName = "Thumbnail Generator Descriptor Set Layout",
		.bindings = layoutBindings.data(),
		.bindingCount = static_cast<uint32_t>(layoutBindings.size()),
	};
	thumbnailGeneratorContext.descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

	Grindstone::GraphicsAPI::Sampler::CreateInfo samplerCreateInfo{
		.debugName = "Thumbnail Generator Sampler",
		.options = {
			.wrapModeU = GraphicsAPI::TextureWrapMode::ClampToBorder,
			.wrapModeV = GraphicsAPI::TextureWrapMode::ClampToBorder,
			.wrapModeW = GraphicsAPI::TextureWrapMode::ClampToBorder,
			.mipFilter = GraphicsAPI::TextureFilter::Nearest,
			.minFilter = GraphicsAPI::TextureFilter::Linear,
			.magFilter = GraphicsAPI::TextureFilter::Linear,
			.anistropy = 0.0f,
			.mipMin = -1000.f,
			.mipMax = 1000.0f,
			.mipBias = 0.0f
		}
	};
	thumbnailGeneratorContext.sampler = graphicsCore->CreateSampler(samplerCreateInfo);

	std::array<Grindstone::GraphicsAPI::DescriptorSet::Binding, 2> descriptorSetBindings = {
		Grindstone::GraphicsAPI::DescriptorSet::Binding::Sampler(thumbnailGeneratorContext.sampler, 1),
		Grindstone::GraphicsAPI::DescriptorSet::Binding::SampledImage(nullptr, 1)
	};
	Grindstone::GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
	descriptorSetCreateInfo.debugName = "Thumbnail Generator Descriptor Set";
	descriptorSetCreateInfo.bindings = descriptorSetBindings.data();
	descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetBindings.size());
	descriptorSetCreateInfo.layout = thumbnailGeneratorContext.descriptorSetLayout;
	thumbnailGeneratorContext.descriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);

	return true;
}

void Grindstone::Editor::Importers::ReleaseTextureThumbnailGenerator() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	thumbnailGeneratorContext.tex2dGraphicsPipelineAsset.Release();
	thumbnailGeneratorContext.cubemapGraphicsPipelineAsset.Release();
	graphicsCore->DeleteDescriptorSet(thumbnailGeneratorContext.descriptorSet);
	graphicsCore->DeleteDescriptorSetLayout(thumbnailGeneratorContext.descriptorSetLayout);
	graphicsCore->DeleteSampler(thumbnailGeneratorContext.sampler);
	graphicsCore->DeleteCommandBuffer(thumbnailGeneratorContext.commandBuffer);
	graphicsCore->DeleteFramebuffer(thumbnailGeneratorContext.framebuffer);
	graphicsCore->DeleteImage(thumbnailGeneratorContext.renderTargetImage);
	graphicsCore->DeleteRenderPass(thumbnailGeneratorContext.renderpass);
}
