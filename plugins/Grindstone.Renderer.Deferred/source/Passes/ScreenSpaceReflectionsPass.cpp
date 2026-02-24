#include <EngineCore/Assets/AssetManager.hpp>

#include <Common/Graphics/DescriptorSet.hpp>
#include <Common/Graphics/Core.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/ScreenSpaceReflectionsPass.hpp>

bool Grindstone::Renderer::ScreenSpaceReflectionsPass::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	ssrPipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<ComputePipelineAsset>("@CORESHADERS/postProcessing/screenSpaceReflections");

	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	GraphicsAPI::DescriptorSetLayout::Binding sourceBinding{};
	sourceBinding.count = 1;
	sourceBinding.type = GraphicsAPI::BindingType::SampledImage;
	sourceBinding.stages = GraphicsAPI::ShaderStageBit::Compute;

	std::array<GraphicsAPI::DescriptorSetLayout::Binding, 7> ssrLayoutBindings{};
	for (size_t i = 0; i < ssrLayoutBindings.size(); ++i) {
		ssrLayoutBindings[i] = sourceBinding;
		ssrLayoutBindings[i].bindingId = static_cast<uint32_t>(i);
	}

	ssrLayoutBindings[0].type = GraphicsAPI::BindingType::UniformBuffer;
	ssrLayoutBindings[1].type = GraphicsAPI::BindingType::Sampler;
	ssrLayoutBindings[2].type = GraphicsAPI::BindingType::StorageImage;

	GraphicsAPI::DescriptorSetLayout::CreateInfo ssrDescriptorSetLayoutCreateInfo{};
	ssrDescriptorSetLayoutCreateInfo.debugName = "SSR Descriptor Set Layout";
	ssrDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ssrLayoutBindings.size());
	ssrDescriptorSetLayoutCreateInfo.bindings = ssrLayoutBindings.data();
	ssrDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ssrDescriptorSetLayoutCreateInfo);

	return true;
}

void Grindstone::Renderer::ScreenSpaceReflectionsPass::AddPass(Grindstone::Renderer::RenderGraph& renderGraph) {
	Grindstone::ComputePipelineAsset* ssrPipelineAsset = ssrPipelineSet.Get();
	if (ssrPipelineAsset == nullptr) {
		return;
	}

	Grindstone::GraphicsAPI::ComputePipeline* ssrPipeline = ssrPipelineAsset->GetPipeline();
	if (ssrPipeline == nullptr) {
		return;
	}

	/*
	struct GbufferData {
		Renderer::RenderGraphResource albedo;
		Renderer::RenderGraphResource normal;
		Renderer::RenderGraphResource specularRoughness;
		Renderer::RenderGraphResource depth;
	};

	Grindstone::Renderer::RenderGraphPass& gbufferPass = renderGraph.AddCallbackPass<GbufferData>(
		"Screen Space Reflections Pass",
		[&](Renderer::RenderGraph::Builder& builder, GbufferData& data) {
			gbufferPass.AddOutputImage(attachmentNameAlbedo, attachmentAlbedo);
			gbufferPass.AddOutputImage(attachmentNameNormal, attachmentNormal);
			gbufferPass.AddOutputImage(attachmentNameSpecularRoughness, attachmentSpecularRoughness);
			gbufferPass.AddOutputImage(attachmentNameDepthStencil, attachmentDepthStencil);
		},
		[&, registry = &registry](const GbufferData& data, Renderer::RenderGraphResource& resources) {
			currentCommandBuffer->BeginDebugLabelSection("Screen Space Reflections Pass", nullptr);
			currentCommandBuffer->BindComputePipeline(ssrPipeline);

			{
				GraphicsAPI::ImageBarrier barrier{
					.image = imageSet.ssrRenderTarget,
					.oldLayout = GraphicsAPI::ImageLayout::Undefined,
					.newLayout = GraphicsAPI::ImageLayout::General,
					.srcAccess = GraphicsAPI::AccessFlags::None,
					.dstAccess = GraphicsAPI::AccessFlags::ShaderWrite,
					.imageAspect = GraphicsAPI::ImageAspectBits::Color,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				};

				currentCommandBuffer->PipelineBarrier(
					GraphicsAPI::PipelineStageBit::TopOfPipe,
					GraphicsAPI::PipelineStageBit::ComputeShader,
					nullptr, 0,
					&barrier, 1
				);

				std::array<Grindstone::GraphicsAPI::DescriptorSet*, 3> ssrDescriptors = {
					imageSet.engineDescriptorSet,
					imageSet.gbufferDescriptorSet,
					imageSet.ssrDescriptorSet
				};

				currentCommandBuffer->BindComputeDescriptorSet(ssrPipeline, &imageSet.ssrDescriptorSet, 2u, 1u);
				currentCommandBuffer->DispatchCompute(renderArea.GetWidth(), renderArea.GetHeight(), 1);

				barrier.image = imageSet.ssrRenderTarget;
				barrier.oldLayout = GraphicsAPI::ImageLayout::General;
				barrier.newLayout = GraphicsAPI::ImageLayout::ShaderRead;
				barrier.srcAccess = GraphicsAPI::AccessFlags::ShaderWrite;
				barrier.dstAccess = GraphicsAPI::AccessFlags::ShaderRead;

				currentCommandBuffer->PipelineBarrier(
					GraphicsAPI::PipelineStageBit::ComputeShader,
					GraphicsAPI::PipelineStageBit::ComputeShader,
					nullptr, 0,
					&barrier, 1
				);
			}
			currentCommandBuffer->EndDebugLabelSection();
		}
	);
	*/
}
