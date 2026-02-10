#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/BlurPass.hpp>

bool Grindstone::Renderer::BlurPass::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	blurPipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/blur");
	return true;
}

void Grindstone::Renderer::BlurPass::AddPass(Grindstone::Renderer::RenderGraph& renderGraph) {
}

/*
	struct GbufferData {
		Renderer::RenderGraphResource albedo;
		Renderer::RenderGraphResource normal;
		Renderer::RenderGraphResource specularRoughness;
		Renderer::RenderGraphResource depth;
	};

	Grindstone::Renderer::RenderGraphPass& gbufferPass = renderGraph.AddCallbackPass<GbufferData>(
		"Gbuffer Geometry Pass",
		[&](Renderer::RenderGraph::Builder& builder, GbufferData& data) {
			gbufferPass.AddOutputImage(attachmentNameAlbedo, attachmentAlbedo);
			gbufferPass.AddOutputImage(attachmentNameNormal, attachmentNormal);
			gbufferPass.AddOutputImage(attachmentNameSpecularRoughness, attachmentSpecularRoughness);
			gbufferPass.AddOutputImage(attachmentNameDepthStencil, attachmentDepthStencil);
		},
		[&, registry = &registry](const GbufferData& data, Renderer::RenderGraphResource& resources) {
			std::array<GraphicsAPI::RenderAttachment, 3> renderAttachments = {
				resources.Get<RenderGraphImage>(data.albedo).image,
				resources.Get<RenderGraphImage>(data.normal).image,
				resources.Get<RenderGraphImage>(data.specularRoughness).image
			};

			commandBuffer->BeginRendering(
				"Gbuffer Geometry Pass",
				renderArea,
				renderAttachments.data(),
				static_cast<uint32_t>(renderAttachments.size()),
				&resources.Get<RenderGraphImage>(data.depth).image
			);

			commandBuffer->SetViewport(0.0f, 0.0f, static_cast<float>(renderArea.GetWidth()), static_cast<float>(renderArea.GetHeight()));
			commandBuffer->SetScissor(0, 0, renderArea.GetWidth(), renderArea.GetHeight());

			// TODO: Get Rendering Stats
			assetManager->RenderQueue(commandBuffer, renderViewData, *registry, geometryOpaqueRenderPassKey);
			commandBuffer->EndRendering();
		}
	);
}

	Grindstone::GraphicsPipelineAsset* blurPipelineSetAsset = ssaoBlurPipelineSet.Get();
	if (blurPipelineSetAsset == nullptr) {
		return;
	}

	Grindstone::GraphicsAPI::GraphicsPipeline* blurPipeline = blurPipelineSetAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
	if (blurPipeline == nullptr) {
		return;
	}

	commandBuffer->BeginRendering(
		"SSAO Blur Pass (Screen-Space Ambient Occlusion)",
		halfRenderArea,
		&imageSet.blurredAmbientOcclusionAttachment,
		1u,
		nullptr,
		nullptr
	);

	commandBuffer->SetViewport(
		static_cast<float>(halfRenderArea.offset.x),
		static_cast<float>(halfRenderArea.offset.y),
		static_cast<float>(halfRenderArea.GetWidth()),
		static_cast<float>(halfRenderArea.GetHeight()),
		0.0f, 1.0f
	);
	commandBuffer->SetScissor(
		halfRenderArea.offset.x, halfRenderArea.offset.y,
		halfRenderArea.GetWidth(), halfRenderArea.GetHeight()
	);

	commandBuffer->BindVertexBuffers(&vertexBuffer, 1);
	commandBuffer->BindIndexBuffer(indexBuffer);

	std::array<Grindstone::GraphicsAPI::DescriptorSet*, 2> blurDescriptorSets = {
		imageSet.engineDescriptorSet,
		imageSet.blurredSsaoInputDescriptorSet
	};

	commandBuffer->BindGraphicsPipeline(blurPipeline);
	commandBuffer->BindGraphicsDescriptorSet(blurPipeline, blurDescriptorSets.data(), 0, static_cast<uint32_t>(blurDescriptorSets.size()));
	commandBuffer->DrawIndices(0, 6, 0, 1, 0);
	commandBuffer->EndRendering();

	*/
