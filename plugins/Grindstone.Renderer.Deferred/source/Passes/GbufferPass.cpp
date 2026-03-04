#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/GbufferPass.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>

bool Grindstone::Renderer::GbufferPass::Initialize() {
	return true;
}

Grindstone::Renderer::GbufferData Grindstone::Renderer::GbufferPass::AddPass(glm::mat4& projectionMatrix, glm::mat4 viewMatrix, Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder) {
	using namespace Grindstone::GraphicsAPI;

	return renderGraphBuilder.CreateGraphicsPass<Grindstone::Renderer::GbufferData>(
		"Gbuffer Geometry Opaque"_hash,
		[](Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::GbufferData>& renderPass) -> Grindstone::Renderer::GbufferData {
			TGBImageRef albedoRef = renderPass.WriteColorAttachment(attachmentAlbedo, ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
			TGBImageRef normalRef = renderPass.WriteColorAttachment(attachmentNormal, ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
			TGBImageRef specularRoughnessRef = renderPass.WriteColorAttachment(attachmentSpecularRoughness, ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
			TGBImageRef depthRef = renderPass.WriteDepthStencilAttachment(attachmentDepthStencil, ClearDepthStencil(1.0f, 0u));

			return Grindstone::Renderer::GbufferData{
				.albedoRef = albedoRef,
				.normalRef = normalRef,
				.specularRoughnessRef = specularRoughnessRef,
				.depthRef = depthRef,
			};
		},
		[projectionMatrix, viewMatrix](
			const Renderer::RenderGraphContext& cxt,
			Renderer::GraphicsRenderGraphPass<Grindstone::Renderer::GbufferData>& renderPassExecution,
			Grindstone::Renderer::GbufferData& data
		) {
			Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
			Grindstone::WorldContextSet* cxtSet = cxt.worldContextSet;
			CommandBuffer* cmd = cxt.commandBuffer;

			Grindstone::Rendering::RenderViewData renderViewData{
				.projectionMatrix = projectionMatrix,
				.viewMatrix = viewMatrix,
				.renderArea = cxt.swapchainSize
			};

			cmd->SetViewport(0.0f, 0.0f, static_cast<float>(renderViewData.renderArea.GetWidth()), static_cast<float>(renderViewData.renderArea.GetHeight()));
			cmd->SetScissor(0, 0, renderViewData.renderArea.GetWidth(), renderViewData.renderArea.GetHeight());

			// TODO: Get Rendering Stats
			engineCore.assetRendererManager->RenderQueue(cmd, renderViewData, cxtSet->GetEntityRegistry(), geometryOpaqueRenderPassKey);
		}
	);
}
