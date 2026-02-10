#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/GbufferPass.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>

bool Grindstone::Renderer::GbufferPass::Initialize() {
	return true;
}

void Grindstone::Renderer::GbufferPass::AddPass(glm::mat4& projectionMatrix, glm::mat4 viewMatrix, Grindstone::Renderer::RenderGraph& renderGraph) {
	renderGraph.AddGraphicsPass(
		"Gbuffer Geometry Opaque",
		[](Renderer::RenderGraph::RenderPass& renderPass) {
			renderPass.WriteColorAttachment(attachmentNameAlbedo, attachmentAlbedo, Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
			renderPass.WriteColorAttachment(attachmentNameNormal, attachmentNormal, Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
			renderPass.WriteColorAttachment(attachmentNameSpecularRoughness, attachmentSpecularRoughness, Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
			renderPass.WriteDepthStencilAttachment(attachmentNameDepthStencil, attachmentDepthStencil, Grindstone::GraphicsAPI::ClearDepthStencil(1.0f, 0u));
		},
		[projectionMatrix, viewMatrix](const Renderer::RenderGraph::RenderGraphContext& cxt, Renderer::RenderGraph::RenderPassExecution& renderPassExecution) {
			Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
			Grindstone::WorldContextSet* cxtSet = cxt.worldContextSet;
			Grindstone::GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;

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
