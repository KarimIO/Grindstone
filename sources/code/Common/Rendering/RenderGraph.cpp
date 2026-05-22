#include <Common/Rendering/RenderGraph.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Window/WindowManager.hpp>
#include <EngineCore/EngineCore.hpp>

#include "RenderGraphFrameResources.hpp"

// ===============================================================
// - RenderPassExecution
// ===============================================================

Grindstone::Renderer::RenderGraph::RenderGraph(
	std::vector<Grindstone::UniquePtr<Grindstone::Renderer::RenderGraphPass>>&& passes,
	const std::vector<UnionResourceDescription>& resourceDescriptions
) : passes(std::move(passes)), resourceDescriptions(resourceDescriptions) {
}

void Grindstone::Renderer::RenderGraph::ExecuteGraph(Grindstone::Renderer::RenderGraphContext context) {
	auto resourceManager = context.transientResourceManager;
	resourceManager->BeginFrame();

	// Build this frame's ResourceId -> physical mapping
	Grindstone::Renderer::RenderGraphFrameResources frameResources;

	for (ResourceId id = 0; id < resourceDescriptions.size(); ++id) {
		const Grindstone::Renderer::UnionResourceDescription& entry = resourceDescriptions[id];

		if (std::holds_alternative<Grindstone::Renderer::ImageDescription>(entry)) {
			const Grindstone::Renderer::ImageDescription& desc = std::get<Grindstone::Renderer::ImageDescription>(entry);
			if (desc.externalGetterCallback != nullptr) {
				frameResources.imageKeys[id] = Grindstone::Renderer::TransientImageKey{
					.poolIndex = SIZE_MAX
				};
				frameResources.externalImages[id] = Grindstone::Renderer::TransientImageData{
					.image = desc.externalGetterCallback(),
					.currentLayout = desc.externalInitialLayout,
					.currentAccessFlags = desc.externalInitialAccessFlags,
					.currentPipelineStage = desc.externalInitialPipelineStage,
				};
			}
			else {
				// TODO: This is godawful. Figure out where ids come from in the builder and pass it here.
				Grindstone::Math::Uint2 viewport = context.swapchainSize;
				bool foundPass = false;
				for (auto& pass : passes) {
					if (pass->type == GpuPassType::Graphics) {
						Renderer::PipelineRenderGraphPass* pipelinePass = static_cast<Renderer::PipelineRenderGraphPass*>(pass.Get());
						for (auto& p : pipelinePass->imageDescs) {
							if (p.IsWrite() && id == p.ref.resourceIndex) {
								foundPass = true;
								Renderer::GraphicsRenderGraphPassBase* graphicsPipelinePass = static_cast<Renderer::GraphicsRenderGraphPassBase*>(pipelinePass);
								viewport = graphicsPipelinePass->metaRenderingArea.Resolve(context.swapchainSize).extent;
								break;
							}
						}

						if (foundPass) {
							break;
						}
					}
				}
				TransientImageKey key = resourceManager->AcquireImage(viewport, context.swapchainSize, desc);
				frameResources.imageKeys[id] = key;
			}
		}
		else {
			const Grindstone::Renderer::BufferDescription& desc = std::get<Grindstone::Renderer::BufferDescription>(entry);
			TransientBufferKey key = resourceManager->AcquireBuffer(desc);
			frameResources.bufferKeys[id] = key;
		}
	}

	frameResources.RealizeKeys(context.transientResourceManager);

	for (auto& pass : passes) {
		pass->RealizeResources(context, frameResources);
		pass->Execute(context, frameResources);
	}

	std::vector<GraphicsAPI::ImageBarrier> finalImageBarriers;
	std::vector<GraphicsAPI::BufferBarrier> finalBufferBarriers;

	for (ResourceId id = 0; id < resourceDescriptions.size(); ++id) {
		const Grindstone::Renderer::UnionResourceDescription& entry = resourceDescriptions[id];

		if (std::holds_alternative<Grindstone::Renderer::ImageDescription>(entry)) {
			const Grindstone::Renderer::ImageDescription& desc = std::get<Grindstone::Renderer::ImageDescription>(entry);
			if (desc.externalGetterCallback != nullptr) {
				auto& ext = frameResources.externalImages[id];

				Grindstone::GraphicsAPI::ImageAspectBits imageAspect = Any(GraphicsAPI::GetFormatDepthStencilType(ext.image->GetFormat()) & GraphicsAPI::FormatDepthStencilType::Depth)
					? Grindstone::GraphicsAPI::ImageAspectBits::Depth
					: Grindstone::GraphicsAPI::ImageAspectBits::Color;

				Grindstone::GraphicsAPI::ImageBarrier imageBarrier{
					.image = ext.image,
					.srcStageMask = ext.currentPipelineStage,
					.dstStageMask = desc.externalFinalPipelineStage,
					.oldLayout = ext.currentLayout,
					.newLayout = desc.externalFinalLayout,
					.srcAccess = ext.currentAccessFlags,
					.dstAccess = desc.externalFinalAccessFlags,
					.imageAspect = imageAspect,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				};

				ext.currentPipelineStage = desc.externalFinalPipelineStage;
				ext.currentLayout = desc.externalFinalLayout;
				ext.currentAccessFlags = desc.externalFinalAccessFlags;

				finalImageBarriers.emplace_back(imageBarrier);
			}
		}
	}

	context.commandBuffer->PipelineBarrier(
		finalBufferBarriers.data(),
		static_cast<uint32_t>(finalBufferBarriers.size()),
		finalImageBarriers.data(),
		static_cast<uint32_t>(finalImageBarriers.size())
	);
}
