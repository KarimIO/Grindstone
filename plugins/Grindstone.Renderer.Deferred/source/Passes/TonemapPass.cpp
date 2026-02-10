#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/TonemapPass.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>

struct PostProcessUbo {
	glm::vec4 vignetteColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	float vignetteRadius = 0.75f;
	float vignetteSoftness = 0.8f;
	float grainAmount = 0.0007f;
	float grainPixelSize = 1.0f;
	glm::vec2 chromaticDistortionRedOffset = glm::vec2(0.00045f, 0.00045f);
	glm::vec2 chromaticDistortionGreenOffset = glm::vec2(0.0003f, 0.0003f);
	glm::vec2 chromaticDistortionBlueOffset = glm::vec2(-0.0003f, -0.0003f);
	float paniniDistortionStrength = 0.0f;
	bool isAnimated = true;

};

bool Grindstone::Renderer::TonemapPass::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	tonemapPipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/tonemapping");
}

void Grindstone::Renderer::TonemapPass::AddPass(Grindstone::Renderer::RenderGraph& renderGraph) {
	renderGraph.AddGraphicsPass(
		"Tonemapping",
		[](Renderer::RenderGraph::RenderPass& renderPass) {
			renderPass.ReadColorAttachment(attachmentNameAlbedo, attachmentAlbedo);
		},
		[&tonemapPipelineSet = tonemapPipelineSet](const Renderer::RenderGraph::RenderGraphContext& cxt, Renderer::RenderGraph::RenderPassExecution& renderPassExecution) {
			Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
			Grindstone::WorldContextSet* cxtSet = cxt.worldContextSet;
			Grindstone::GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;

			Grindstone::GraphicsPipelineAsset* tonemapPipelineAsset = tonemapPipelineSet.Get();
			if (tonemapPipelineAsset == nullptr) {
				return;
			}

			Grindstone::GraphicsAPI::GraphicsPipeline* tonemapPipeline = tonemapPipelineAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
			if (tonemapPipeline == nullptr) {
				return;
			}

			tonemapPostProcessingUniformBufferObject->UploadData(&postProcessUboData);

			std::array<Grindstone::GraphicsAPI::DescriptorSet*, 3> descriptorSets{};
			descriptorSets[0] = engineDescriptorSet;
			descriptorSets[1] = gbufferDescriptorSet;
			descriptorSets[2] = tonemapDescriptorSet;

			cmd->BindGraphicsPipeline(tonemapPipeline);
			cmd->BindGraphicsDescriptorSet(tonemapPipeline, descriptorSets.data(), 0, static_cast<uint32_t>(descriptorSets.size()));
			cmd->DrawIndices(0, 6, 0, 1, 0);
		}
	);
}
