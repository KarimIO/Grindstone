#include <EngineCore/Assets/AssetManager.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/DebugPass.hpp>

bool Grindstone::Renderer::DebugPass::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	debugPipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/editor/debug");
	return true;
}

void Grindstone::Renderer::DebugPass::AddPass(Grindstone::Renderer::RenderGraph& renderGraph) {
}

/*
struct DebugUboData {
	uint32_t renderMode;
	float nearDistance;
	float farDistance;
};

DebugUboData debugUboData;

void DrawGbufferPass() {
	debugUboData.renderMode = static_cast<uint32_t>(renderMode);
	float projection_43 = projectionMatrix[3][2];
	float projection_33 = projectionMatrix[2][2];
	debugUboData.nearDistance = projection_43 / (projection_33 - 1.0f);
	debugUboData.farDistance = projection_43 / (projection_33 + 1.0f);

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
			currentCommandBuffer->BeginRendering(
				"Debug Pass",
				renderArea,
				&outRenderAttachment,
				1u
			);

			Grindstone::GraphicsAPI::GraphicsPipeline* debugPipeline = debugPipelineSet.Get()->GetFirstPassPipeline(&vertexLightPositionLayout);
			if (debugPipeline != nullptr) {
				imageSet.debugUniformBufferObject->UploadData(&debugUboData);

				currentCommandBuffer->BindGraphicsDescriptorSet(debugPipeline, &imageSet.debugDescriptorSet, 2, 1);
				currentCommandBuffer->BindGraphicsPipeline(debugPipeline);
				currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
			}

			currentCommandBuffer->EndRendering();
		}
	);
}
*/
