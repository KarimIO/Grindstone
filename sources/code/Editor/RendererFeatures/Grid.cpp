#include <glm/gtx/transform.hpp>

#include <Common/Graphics/Buffer.hpp>
#include <Common/Window/WindowManager.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <Editor/RendererFeatures/Grid.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>

struct GridUniformBuffer {
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 inverseProjectionMatrix;
	glm::mat4 inverseViewMatrix;
	glm::vec4 colorXAxis = glm::vec4(1.0f, 0.2f, 0.2f, 1.0f);
	glm::vec4 colorZAxis = glm::vec4(0.2f, 0.2f, 1.0f, 1.0f);
	glm::vec4 colorMinor = glm::vec4(0.2f, 0.2f, 0.2f, 0.5f);
	glm::vec4 colorMajor = glm::vec4(0.2f, 0.2f, 0.2f, 0.9f);
	glm::vec2 renderScale;
	float fadeDistanceMultiplier = 0.0f;
	float nearDistance;
	float farDistance;
};

void Grindstone::Editor::RendererFeatures::Grid::Initialize() {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	GraphicsAPI::Buffer::CreateInfo ubCi{
		.bufferSize = static_cast<size_t>(sizeof(GridUniformBuffer)),
		.bufferUsage =
			GraphicsAPI::BufferUsage::TransferDst |
			GraphicsAPI::BufferUsage::TransferSrc |
			GraphicsAPI::BufferUsage::Uniform,
		.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU
	};

	for (uint32_t i = 0; i < static_cast<uint32_t>(gridUniformBuffers.size()); ++i) {
		std::string debugName = std::vformat("Grid Uniform Buffer [{}]", std::make_format_args(i));
		ubCi.debugName = debugName.c_str();
		gridUniformBuffers[i] = graphicsCore->CreateBuffer(ubCi);
	}

	std::vector<GraphicsAPI::GraphicsPipeline::ShaderStageData> shaderStageCreateInfos;
	std::vector<std::vector<char>> fileData;

	Grindstone::Assets::AssetManager* assetManager = engineCore.assetManager;
	pipelineSet = assetManager->GetAssetReferenceByAddress<Grindstone::GraphicsPipelineAsset>("@CORESHADERS/editor/grid");

	GraphicsAPI::DescriptorSetLayout::Binding gridDescriptorLayoutBinding {
		0,
		1,
		GraphicsAPI::BindingType::UniformBuffer,
		GraphicsAPI::ShaderStageBit::Vertex | GraphicsAPI::ShaderStageBit::Fragment
	};

	GraphicsAPI::DescriptorSetLayout::CreateInfo gridDescriptorSetLayoutCreateInfo{
		.debugName = "Grid Descriptor Layout",
		.bindings = &gridDescriptorLayoutBinding,
		.bindingCount = 1u,
	};
	gridDescriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(gridDescriptorSetLayoutCreateInfo);

	GraphicsAPI::DescriptorSet::CreateInfo gridDescriptorSetCreateInfo{
		.layout = gridDescriptorSetLayout,
		.bindingCount = 1u,
	};

	for (uint32_t i = 0; i < static_cast<uint32_t>(gridDescriptorSets.size()); ++i) {
		std::string debugName = std::vformat("Grid Descriptor [{}]", std::make_format_args(i));
		gridDescriptorSetCreateInfo.debugName = debugName.c_str();
		GraphicsAPI::DescriptorSet::Binding gridDescriptorBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(gridUniformBuffers[i]);
		gridDescriptorSetCreateInfo.bindings = &gridDescriptorBinding;
		gridDescriptorSets[i] = graphicsCore->CreateDescriptorSet(gridDescriptorSetCreateInfo);
	}
}

void Grindstone::Editor::RendererFeatures::Grid::Bind(Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder) {
	Renderer::RenderGraphBuilderResourceRef gridImageRef = renderGraphBuilder.CreateGraphicsPass<Renderer::RenderGraphBuilderResourceRef>(
		"Grid Pass",
		Renderer::MetaRect::Swapchain(),
		[](Renderer::GraphicsRenderGraphBuilderPass<Renderer::RenderGraphBuilderResourceRef>& pass) -> Renderer::RenderGraphBuilderResourceRef {
			Renderer::RenderGraphBuilderResourceRef outputRef = pass.ReadWriteColorAttachment(colorImageRef);
			pass.ReadDepthAttachment(depthImageRef);
			return outputRef;
		},
		[this](
			Grindstone::Math::IntRect2D rect,
			const Grindstone::Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			Renderer::RenderGraphBuilderResourceRef& outputRef
		) {
			// TODO: RenderGraph2 - link values
			glm::mat4 proj;
			glm::mat4 view;
			glm::vec2 renderScale;
			float renderScale, nearDist, farDist;

			EngineCore& engineCore = EngineCore::GetInstance();
			GraphicsAPI::Core* graphicsCore = cxt.graphicsCore;
			GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;
			uint32_t imageIndex = cxt.swapchainIndex;

			Grindstone::GraphicsPipelineAsset* pipelineAsset = pipelineSet.Get();
			if (pipelineAsset == nullptr) {
				return;
			}

			Grindstone::GraphicsAPI::GraphicsPipeline* pipeline = pipelineAsset->GetFirstPassPipeline(nullptr);
			if (pipeline == nullptr) {
				return;
			}

			Grindstone::GraphicsAPI::PipelineLayout* pipelineLayout = pipelineAsset->GetFirstPassPipelineLayout();

			GridUniformBuffer gridData{
				.projectionMatrix = proj,
				.viewMatrix = view,
				.inverseProjectionMatrix = glm::inverse(proj),
				.inverseViewMatrix = glm::inverse(view),
				.renderScale = renderScale,
				.nearDistance = nearDist,
				.farDistance = farDist
			};

			gridUniformBuffers[imageIndex]->UploadData(&gridData);
			cmd->BindGraphicsPipeline(pipeline);
			cmd->BindGraphicsDescriptorSet(pipelineLayout, &gridDescriptorSets[imageIndex], 2, 1);
			cmd->DrawVertices(6, 0, 1, 0);
		}
	);
}
