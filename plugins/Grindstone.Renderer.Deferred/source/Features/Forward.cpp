#include <Grindstone.Renderer.Deferred/include/Features/Forward.hpp>

void Grindstone::Renderer::Forward::Initialize() {
}

void Grindstone::Renderer::Forward::Bind(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderFrameContext& context
) {
}

/*
void DrawGbufferPass() {
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
*/
