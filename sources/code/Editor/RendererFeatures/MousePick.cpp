#include <Common/Graphics/Buffer.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/Logger.hpp>

#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <Editor/RendererFeatures/MousePick.hpp>

struct MousePickMatrixBuffer {
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
};

struct MousePickResponseBuffer {
	float depth;
	uint32_t entityId;
};

static Grindstone::Renderer::ImageDescription resource{
	.name = "Mouse Pick Color Attachment",
	.size = Grindstone::Renderer::MetaSize2D::Viewport(),
	.samples = 1,
	.mipLevels = 1,
	.depth = 1,
	.arrayLayers = 1,
	.format = Grindstone::GraphicsAPI::Format::R32_UINT,
	.imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension2D,
	.memoryUsage = Grindstone::GraphicsAPI::MemoryUsage::GPUOnly,
	.imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::Sampled | Grindstone::GraphicsAPI::ImageUsageFlags::RenderTarget
};

static Grindstone::Renderer::ImageDescription depthResourceDesc{
	.name = "Mouse Pick Depth Attachment",
	.size = Grindstone::Renderer::MetaSize2D::Viewport(),
	.samples = 1,
	.mipLevels = 1,
	.depth = 1,
	.arrayLayers = 1,
	.format = Grindstone::GraphicsAPI::Format::D32_SFLOAT,
	.imageDimensions = Grindstone::GraphicsAPI::ImageDimension::Dimension2D,
	.memoryUsage = Grindstone::GraphicsAPI::MemoryUsage::GPUOnly,
	.imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::Sampled | Grindstone::GraphicsAPI::ImageUsageFlags::DepthStencil
};

void Grindstone::Editor::RendererFeatures::MousePick::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	mousePickPipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/screenSpaceAmbientOcclusionBlur");

	{
		Grindstone::GraphicsAPI::Buffer::CreateInfo mousePickBufferMatrixCreateInfo{};
		mousePickBufferMatrixCreateInfo.bufferSize = sizeof(MousePickMatrixBuffer);
		mousePickBufferMatrixCreateInfo.bufferUsage =
			GraphicsAPI::BufferUsage::TransferDst |
			GraphicsAPI::BufferUsage::TransferSrc |
			GraphicsAPI::BufferUsage::Uniform;
		mousePickBufferMatrixCreateInfo.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
		mousePickBufferMatrixCreateInfo.content = nullptr;

		MousePickResponseBuffer mousePickResponseInitialBuffer{};
		mousePickResponseInitialBuffer.depth = 1.0f;
		mousePickResponseInitialBuffer.entityId = static_cast<uint32_t>(entt::null);

		Grindstone::GraphicsAPI::Buffer::CreateInfo mousePickBufferResponseCreateInfo{};
		mousePickBufferResponseCreateInfo.bufferSize = sizeof(MousePickResponseBuffer);
		mousePickBufferResponseCreateInfo.memoryUsage = Grindstone::GraphicsAPI::MemoryUsage::CPUOnly;
		mousePickBufferResponseCreateInfo.bufferUsage = Grindstone::GraphicsAPI::BufferUsage::Storage | GraphicsAPI::BufferUsage::TransferSrc | Grindstone::GraphicsAPI::BufferUsage::TransferDst;
		mousePickBufferResponseCreateInfo.content = &mousePickResponseInitialBuffer;

		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 2> mousePickDescriptorBindingLayouts{};
		mousePickDescriptorBindingLayouts[0] = GraphicsAPI::DescriptorSetLayout::Binding{ 0, 1, GraphicsAPI::BindingType::UniformBuffer, GraphicsAPI::ShaderStageBit::Vertex | GraphicsAPI::ShaderStageBit::Fragment };
		mousePickDescriptorBindingLayouts[1] = GraphicsAPI::DescriptorSetLayout::Binding{ 1, 1, GraphicsAPI::BindingType::StorageBuffer, GraphicsAPI::ShaderStageBit::Fragment };

		GraphicsAPI::DescriptorSetLayout::CreateInfo mousePickDescriptorSetLayoutCreateInfo{};
		mousePickDescriptorSetLayoutCreateInfo.debugName = "Mouse Pick Descriptor Set Layout";
		mousePickDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(mousePickDescriptorBindingLayouts.size());
		mousePickDescriptorSetLayoutCreateInfo.bindings = mousePickDescriptorBindingLayouts.data();
		mousePickDescriptorSetLayout = core->GetOrCreateDescriptorSetLayoutFromCache(mousePickDescriptorSetLayoutCreateInfo);

		GraphicsAPI::DescriptorSet::CreateInfo mousePickDescriptorSetCreateInfo{};
		mousePickDescriptorSetCreateInfo.layout = mousePickDescriptorSetLayout;

		for (int i = 0; i < 3; ++i) {
			std::string mousePickMatrixBufferName = std::vformat("Mouse Pick Uniform Buffer [{}]", std::make_format_args(i));
			std::string mousePickResponseBufferName = std::vformat("Mouse Pick SSBO [{}]", std::make_format_args(i));
			std::string mousePickDescriptorSetName = std::vformat("Mouse Pick Descriptor Set [{}]", std::make_format_args(i));

			mousePickBufferMatrixCreateInfo.debugName = mousePickMatrixBufferName.c_str();
			mousePickBufferResponseCreateInfo.debugName = mousePickResponseBufferName.c_str();
			mousePickDescriptorSetCreateInfo.debugName = mousePickDescriptorSetName.c_str();

			mousePickMatrixBuffer[i] = core->CreateBuffer(mousePickBufferMatrixCreateInfo);
			mousePickResponseBuffer[i] = core->CreateBuffer(mousePickBufferResponseCreateInfo);

			std::array<GraphicsAPI::DescriptorSet::Binding, 2> mousePickDescriptorBindings{};
			mousePickDescriptorBindings[0] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(mousePickMatrixBuffer[i]);
			mousePickDescriptorBindings[1] = GraphicsAPI::DescriptorSet::Binding::StorageBuffer(mousePickResponseBuffer[i]);

			mousePickDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(mousePickDescriptorBindings.size());
			mousePickDescriptorSetCreateInfo.bindings = mousePickDescriptorBindings.data();
			mousePickDescriptorSet[i] = core->CreateDescriptorSet(mousePickDescriptorSetCreateInfo);
		}
	}
}

void Grindstone::Editor::RendererFeatures::MousePick::Bind(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderFrameContext& context
) {
	auto litImageResponse = context.blackboard.GetValue<Renderer::RenderGraphBuilderResourceRef>("SceneDepth");
	if (litImageResponse.HasError()) {
		GPRINT_ERROR_V(LogSource::Rendering, "MousePick: Unable to get SceneDepth: {}", litImageResponse.GetError());
		return;
	}

	Grindstone::Renderer::RenderGraphBuilderResourceRef litImageRef = litImageResponse.GetValue();

	renderGraphBuilder.CreateGraphicsPass<Renderer::RenderGraphBuilderResourceRef>(
		"Mouse Pick",
		Renderer::MetaRect::Swapchain(),
		[depthImageRef](Renderer::GraphicsRenderGraphBuilderPass<Renderer::RenderGraphBuilderResourceRef>& pass) -> Renderer::RenderGraphBuilderResourceRef {
			GraphicsAPI::ClearColor clearColor(UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX);
			GraphicsAPI::ClearDepthStencil clearDepthStencil{};
			clearDepthStencil.depth = 1.0f;
			clearDepthStencil.stencil = 0;

			Renderer::RenderGraphBuilderResourceRef outputRef = pass.WriteColorAttachment(resource, GraphicsAPI::LoadOp::Clear, clearColor);
			pass.WriteDepthStencilAttachment(depthResourceDesc, GraphicsAPI::LoadOp::Clear, clearDepthStencil);
			return outputRef;
		},
		[this, adjustedPerspectiveMatrix, pushStats, imageIndex](
			Grindstone::Math::IntRect2D rect,
			const Grindstone::Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			Renderer::RenderGraphBuilderResourceRef& outputRef
		) {
			Grindstone::EngineCore& engineCore = EngineCore::GetInstance();
			Grindstone::AssetRendererManager* assetRendererManager = engineCore.assetRendererManager;
			Grindstone::GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;
			Grindstone::GraphicsAPI::Core* graphicsCore = cxt.graphicsCore;
			entt::registry& registry = cxt.worldContextSet->GetEntityRegistry();
			uint32_t imageIndex = cxt.swapchainIndex;

			MousePickResponseBuffer mousePickResponseInitialBuffer{
				.depth = 1.0f,
				.entityId = static_cast<uint32_t>(entt::null),
			};
			mousePickResponseBuffer[imageIndex]->UploadData(&mousePickResponseInitialBuffer);

			MousePickMatrixBuffer matrixBuffer{
				.projectionMatrix = cxt.cameraViewData.projectionMatrix,
				.viewMatrix = cxt.cameraViewData.viewMatrix,
			};
			mousePickMatrixBuffer[imageIndex]->UploadData(&matrixBuffer);

			cmd->SetViewport(0.0f, 0.0f, static_cast<float>(cxt.cameraViewData.renderArea.GetWidth()), static_cast<float>(cxt.cameraViewData.renderArea.GetHeight()));
			cmd->SetScissor(captureX, captureY, 1, 1);

			assetRendererManager->SetEngineDescriptorSet(mousePickDescriptorSet[imageIndex]);
			pushStats(assetRendererManager->RenderQueue("Editor Mouse Pick", cmd, cxt.viewData, registry, mousePickRenderQueue));
		}
	);
}
