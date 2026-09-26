#include <Common/Console/Cvars.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <Editor/EditorManager.hpp>
#include <Common/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/Rendering/RenderPassRegistry.hpp>

#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Gbuffer.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshDebugRenderFeature.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshDebugRenderer.hpp>

const Grindstone::ConstHashedString navmeshRenderPassHashedString("NavMesh");

void Grindstone::Ai::RendererFeatures::NavMeshDebug::Initialize() {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::RenderPassRegistry* renderPassRegistry = engineCore.GetRenderPassRegistry();

	std::array<GraphicsAPI::RenderPass::AttachmentInfo, 1> navmeshAttachments = { { GraphicsAPI::Format::R8G8B8A8_UNORM, false } };

	GraphicsAPI::RenderPass::CreateInfo navmeshRenderPassCreateInfo{
		.debugName = "Editor Navmesh RenderPass",
		.colorAttachments = navmeshAttachments.data(),
		.colorAttachmentCount = static_cast<uint32_t>(navmeshAttachments.size()),
		.depthFormat = GraphicsAPI::Format::D32_SFLOAT,
		.shouldClearDepthOnLoad = false
	};
	auto navmeshRenderPass = graphicsCore->CreateRenderPass(navmeshRenderPassCreateInfo);
	renderPassRegistry->RegisterRenderpass(navmeshRenderPassHashedString, navmeshRenderPass);

	Grindstone::Assets::AssetManager* assetManager = engineCore.assetManager;
	navmeshPipelineSet = assetManager->GetAssetReferenceByAddress<Grindstone::GraphicsPipelineAsset>("@CORESHADERS/editor/navmesh");
}

void Grindstone::Ai::RendererFeatures::NavMeshDebug::Bind(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderFrameContext& context
) {
	NavMeshDebugRenderer* debugRenderer = NavMeshDebugRenderer::GetInstance();
	if (!debugRenderer->ShouldRenderThisFrame()) {
		return;
	}

	auto gbufferDataResponse = context.blackboard.GetValue<Grindstone::Renderer::GbufferData>();
	if (gbufferDataResponse.HasError()) {
		GPRINT_ERROR(LogSource::Rendering, "NavMeshDebug: Unable to get Gbuffer: {}", gbufferDataResponse.GetError());
		return;
	}
	Grindstone::Renderer::GbufferData gbufferData = gbufferDataResponse.GetValue();

	Grindstone::Renderer::RenderGraphBuilderResourceRef colorImageRef = context.colorRef;
	Grindstone::Renderer::RenderGraphBuilderResourceRef depthImageRef = gbufferData.depthRef;

	renderGraphBuilder.CreateGraphicsPass<Renderer::RenderGraphBuilderResourceRef>(
		"Navmesh Debug Pass",
		Renderer::MetaRect::Swapchain(),
		[colorImageRef, depthImageRef](Renderer::GraphicsRenderGraphBuilderPass<Renderer::RenderGraphBuilderResourceRef>& pass) -> Renderer::RenderGraphBuilderResourceRef {
			Renderer::RenderGraphBuilderResourceRef outputRef = pass.ReadWriteColorAttachment(colorImageRef);
			pass.ReadDepthAttachment(depthImageRef);
			return outputRef;
		},
		[this](
			Grindstone::Math::IntRect2D rect,
			const Grindstone::Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			Renderer::RenderGraphBuilderResourceRef& outputRef
		) {
			Grindstone::GraphicsPipelineAsset* pipelineAsset = navmeshPipelineSet.Get();
			if (pipelineAsset == nullptr) {
				return;
			}

			NavMeshDebugRenderer* debugRenderer = NavMeshDebugRenderer::GetInstance();
			debugRenderer->DrawDebug(cxt.commandBuffer, pipelineAsset);
		}
	);
}
