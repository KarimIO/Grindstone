#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/GbufferPass.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>

bool Grindstone::Renderer::GbufferPass::Initialize() {
	return true;
}

void Grindstone::Renderer::GbufferPass::AddPass(glm::mat4& projectionMatrix, glm::mat4 viewMatrix, Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder) {
	using namespace Grindstone::GraphicsAPI;

	renderGraphBuilder.CreateGraphicsPass<void>(
		"Gbuffer Geometry Opaque"_hash,
		[](Renderer::GraphicsRenderGraphBuilderPass& renderPass) {
			renderPass.WriteColorAttachment(attachmentNameAlbedo, attachmentAlbedo, ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
			renderPass.WriteColorAttachment(attachmentNameNormal, attachmentNormal, ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
			renderPass.WriteColorAttachment(attachmentNameSpecularRoughness, attachmentSpecularRoughness, ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
			renderPass.WriteDepthStencilAttachment(attachmentNameDepthStencil, attachmentDepthStencil, ClearDepthStencil(1.0f, 0u));
		},
		[projectionMatrix, viewMatrix](Grindstone::Renderer::RenderGraphContext& cxt, Grindstone::Renderer::GraphicsRenderGraphPass* pass) {
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
