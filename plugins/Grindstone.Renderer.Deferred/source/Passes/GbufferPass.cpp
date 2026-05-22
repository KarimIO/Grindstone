#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/GbufferPass.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>

bool Grindstone::Renderer::GbufferPass::Initialize() {
	return true;
}

Grindstone::Renderer::GbufferData Grindstone::Renderer::GbufferPass::AddPass(RenderGraphBuilderResourceRef depthImageRef, glm::mat4& projectionMatrix, glm::mat4 viewMatrix, Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder) {
	using namespace Grindstone::GraphicsAPI;

	return renderGraphBuilder.CreateGraphicsPass<Grindstone::Renderer::GbufferData>(
		"Gbuffer Geometry Opaque",
		MetaRect::Swapchain(),
		[depthImageRef](Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::GbufferData>& renderPass) -> Grindstone::Renderer::GbufferData {
			RenderGraphBuilderResourceRef albedoRef = renderPass.WriteColorAttachment(attachmentAlbedo, GraphicsAPI::LoadOp::Clear, ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
			RenderGraphBuilderResourceRef normalRef = renderPass.WriteColorAttachment(attachmentNormal, GraphicsAPI::LoadOp::Clear, ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
			RenderGraphBuilderResourceRef specularRoughnessRef = renderPass.WriteColorAttachment(attachmentSpecularRoughness, GraphicsAPI::LoadOp::Clear, ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
			RenderGraphBuilderResourceRef depthRef = renderPass.WriteDepthStencilAttachment(depthImageRef, GraphicsAPI::LoadOp::Clear, ClearDepthStencil(1.0f, 0u));

			return Grindstone::Renderer::GbufferData{
				.albedoRef = albedoRef,
				.normalRef = normalRef,
				.specularRoughnessRef = specularRoughnessRef,
				.depthRef = depthRef,
			};
		},
		[projectionMatrix, viewMatrix](
			Grindstone::Math::IntRect2D viewportArea,
			const Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			Grindstone::Renderer::GbufferData& data
		) {
			Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
			Grindstone::WorldContextSet* cxtSet = cxt.worldContextSet;
			CommandBuffer* cmd = cxt.commandBuffer;
			engineCore.assetRendererManager->SetEngineDescriptorSet(cxt.globalDescriptorSet);

			Grindstone::Rendering::RenderViewData renderViewData{
				.projectionMatrix = projectionMatrix,
				.viewMatrix = viewMatrix,
				.renderArea = viewportArea
			};

			// TODO: Get Rendering Stats
			engineCore.assetRendererManager->RenderQueue(cmd, renderViewData, cxtSet->GetEntityRegistry(), geometryOpaqueRenderPassKey);
		}
	);
}
