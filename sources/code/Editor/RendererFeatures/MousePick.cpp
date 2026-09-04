#include <Common/Graphics/Buffer.hpp>
#include <Common/Window/WindowManager.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/Rendering/RenderPassRegistry.hpp>

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
	Grindstone::RenderPassRegistry* renderPassRegistry = engineCore.GetRenderPassRegistry();

	{
		Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
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
		mousePickDescriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(mousePickDescriptorSetLayoutCreateInfo);

		GraphicsAPI::DescriptorSet::CreateInfo mousePickDescriptorSetCreateInfo{};
		mousePickDescriptorSetCreateInfo.layout = mousePickDescriptorSetLayout;

		for (int i = 0; i < 3; ++i) {
			std::string mousePickMatrixBufferName = std::vformat("Mouse Pick Uniform Buffer [{}]", std::make_format_args(i));
			std::string mousePickResponseBufferName = std::vformat("Mouse Pick SSBO [{}]", std::make_format_args(i));
			std::string mousePickDescriptorSetName = std::vformat("Mouse Pick Descriptor Set [{}]", std::make_format_args(i));

			mousePickBufferMatrixCreateInfo.debugName = mousePickMatrixBufferName.c_str();
			mousePickBufferResponseCreateInfo.debugName = mousePickResponseBufferName.c_str();
			mousePickDescriptorSetCreateInfo.debugName = mousePickDescriptorSetName.c_str();

			mousePickMatrixBuffer[i] = graphicsCore->CreateBuffer(mousePickBufferMatrixCreateInfo);
			mousePickResponseBuffer[i] = graphicsCore->CreateBuffer(mousePickBufferResponseCreateInfo);

			std::array<GraphicsAPI::DescriptorSet::Binding, 2> mousePickDescriptorBindings{};
			mousePickDescriptorBindings[0] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(mousePickMatrixBuffer[i]);
			mousePickDescriptorBindings[1] = GraphicsAPI::DescriptorSet::Binding::StorageBuffer(mousePickResponseBuffer[i]);

			mousePickDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(mousePickDescriptorBindings.size());
			mousePickDescriptorSetCreateInfo.bindings = mousePickDescriptorBindings.data();
			mousePickDescriptorSet[i] = graphicsCore->CreateDescriptorSet(mousePickDescriptorSetCreateInfo);
		}
	}
}

void Grindstone::Editor::RendererFeatures::MousePick::Bind(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderFrameContext& context
) {
	auto mousePickResponse = context.blackboard.GetValue<Grindstone::Math::Int2>("MousePickCoords");
	if (mousePickResponse.HasError()) {
		if (mousePickResponse.GetError() != Grindstone::Blackboard::BlackboardError::KeyNotFound) {
			GPRINT_ERROR(LogSource::Rendering, "MousePick: Unable to get MousePickCoords: {}", mousePickResponse.GetError());
		}

		return;
	}

	Grindstone::Math::Int2 mousePickCoords = mousePickResponse.GetValue();
	if (mousePickCoords.x < 0 || mousePickCoords.y < 0 ||
		mousePickCoords.x >= context.imageSize.x || mousePickCoords.y >= context.imageSize.y
	) {
		return;
	}

	renderGraphBuilder.CreateGraphicsPass<Renderer::RenderGraphBuilderResourceRef>(
		"Mouse Pick",
		Renderer::MetaRect::Swapchain(),
		[](Renderer::GraphicsRenderGraphBuilderPass<Renderer::RenderGraphBuilderResourceRef>& pass) -> Renderer::RenderGraphBuilderResourceRef {
			GraphicsAPI::ClearColor clearColor(UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX);
			GraphicsAPI::ClearDepthStencil clearDepthStencil{};
			clearDepthStencil.depth = 1.0f;
			clearDepthStencil.stencil = 0;

			Renderer::RenderGraphBuilderResourceRef outputRef = pass.WriteColorAttachment(resource, GraphicsAPI::LoadOp::Clear, clearColor);
			pass.WriteDepthStencilAttachment(depthResourceDesc, GraphicsAPI::LoadOp::Clear, clearDepthStencil);
			return outputRef;
		},
		[this, imageIndex=context.imageIndex, mousePickCoords](
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
			cmd->SetScissor(mousePickCoords.x, mousePickCoords.y, 1, 1);

			assetRendererManager->SetEngineDescriptorSet(mousePickDescriptorSet[imageIndex]);
			Rendering::GeometryRenderStats stats = assetRendererManager->RenderQueue("Editor Mouse Pick", cmd, cxt.cameraViewData, registry, mousePickRenderQueue);
			// TODO: RenderGraph 2.0: pushStats(stats);
		}
	);
}

// TODO: This doesn't make sense - RenderFeatures should not have per-view data, per-view data should be transient, and data
// should be taken through the Blackboard. The problem is this is last-frame data.
uint32_t Grindstone::Editor::RendererFeatures::MousePick::GetMousePickedEntity(GraphicsAPI::CommandBuffer* commandBuffer) {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::WindowGraphicsBinding* wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	uint32_t frameIndex = (wgb->GetCurrentImageIndex() + (wgb->GetMaxFramesInFlight() - 1)) % wgb->GetMaxFramesInFlight();
	Grindstone::GraphicsAPI::Buffer* buffer = mousePickResponseBuffer[frameIndex];

	GraphicsAPI::BufferBarrier bufferBarrier{
		.buffer = buffer,
		.srcStageMask = GraphicsAPI::PipelineStageBit::FragmentShader,
		.dstStageMask = GraphicsAPI::PipelineStageBit::Host,
		.srcAccess = GraphicsAPI::AccessFlags::ShaderWrite,
		.dstAccess = GraphicsAPI::AccessFlags::HostRead,
		.offset = 0,
		.size = static_cast<uint32_t>(buffer->GetSize())
	};

	commandBuffer->PipelineBarrier(
		&bufferBarrier, 1,
		nullptr, 0
	);

	MousePickResponseBuffer* mappedBuffer = reinterpret_cast<MousePickResponseBuffer*>(buffer->Map());
	uint32_t entityId = mappedBuffer->entityId;
	buffer->Unmap();

	return entityId;
}
