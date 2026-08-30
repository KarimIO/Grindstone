#include <EngineCore/Logger.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Renderer.Deferred/include/Features/Gbuffer.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>

void Grindstone::Renderer::Gbuffer::Bind(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderFrameContext& context
) {
	auto depthResult = context.blackboard.GetValue<RenderGraphBuilderResourceRef>("DepthPrePass");
	if (depthResult.HasError()) {
		GPRINT_ERROR_V(LogSource::Rendering, "Gbuffer: Unable to get DepthPrePass: {}", depthResult.GetError());
		return;
	}

	RenderGraphBuilderResourceRef depthImageRef = depthResult.GetValue();

	for (const RenderFrameViewContext& view : context.views) {
		const glm::mat4& projectionMatrix = view.projectionMatrix;
		const glm::mat4& viewMatrix = view.viewMatrix;

		Grindstone::Renderer::GbufferData gbufferData = renderGraphBuilder.CreateGraphicsPass<Grindstone::Renderer::GbufferData>(
			"Gbuffer Geometry Opaque",
			MetaRect::Swapchain(),
			[depthImageRef](Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::GbufferData>& renderPass) -> Renderer::GbufferData {
				RenderGraphBuilderResourceRef albedoRef = renderPass.WriteColorAttachment(attachmentAlbedo, GraphicsAPI::LoadOp::Clear, GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
				RenderGraphBuilderResourceRef normalRef = renderPass.WriteColorAttachment(attachmentNormal, GraphicsAPI::LoadOp::Clear, GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
				RenderGraphBuilderResourceRef specularRoughnessRef = renderPass.WriteColorAttachment(attachmentSpecularRoughness, GraphicsAPI::LoadOp::Clear, GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f));
				RenderGraphBuilderResourceRef depthRef = renderPass.WriteDepthStencilAttachment(depthImageRef, GraphicsAPI::LoadOp::Clear, GraphicsAPI::ClearDepthStencil(1.0f, 0u));

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
				GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;
				engineCore.assetRendererManager->SetEngineDescriptorSet(cxt.globalDescriptorSet);

				Grindstone::Rendering::RenderViewData renderViewData{
					.projectionMatrix = projectionMatrix,
					.viewMatrix = viewMatrix,
					.renderArea = viewportArea
				};

				const Grindstone::Rendering::GeometryRenderStats stats = engineCore.assetRendererManager->RenderQueue("Gbuffer Geometry Opaque", cmd, renderViewData, cxtSet->GetEntityRegistry(), geometryOpaqueRenderPassKey);
				// pushRenderingStatsCallback(stats);
			}
		);

		context.blackboard.SetValue(gbufferData);
	}
}
