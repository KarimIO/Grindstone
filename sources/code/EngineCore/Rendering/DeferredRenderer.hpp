#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include "BaseRenderer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class UniformBuffer;
		class Framebuffer;
		class RenderTarget;
		class DepthStencilTarget;
		class VertexArrayObject;
		class GraphicsPipeline;
		class ComputePipeline;
		class CommandBuffer;
	};

	class DeferredRenderer : public BaseRenderer {
	public:
		DeferredRenderer(GraphicsAPI::RenderPass* targetRenderPass);
		virtual ~DeferredRenderer();
		virtual bool OnWindowResize(Events::BaseEvent*) override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void Render(
			GraphicsAPI::CommandBuffer* commandBuffer,
			entt::registry& registry,
			glm::mat4 projectionMatrix,
			glm::mat4 viewMatrix,
			glm::vec3 eyePos,
			GraphicsAPI::Framebuffer* outputFramebuffer
		) override;

		virtual uint16_t GetRenderModeCount() const override;
		virtual const RenderMode* GetRenderModes() const override;
		virtual void SetRenderMode(uint16_t mode) override;

		static GraphicsAPI::RenderPass* gbufferRenderPass;
		static GraphicsAPI::RenderPass* mainRenderPass;

		enum class DeferredRenderMode : uint16_t {
			Default,
			Position,
			PositionMod,
			ViewPosition,
			ViewPositionMod,
			Depth,
			DepthMod,
			Normal,
			ViewNormal,
			Albedo,
			Specular,
			Roughness,
			AmbientOcclusion,
			Count
		};

	private:
		struct DeferredRendererImageSet {
			GraphicsAPI::Framebuffer* gbuffer = nullptr;
			GraphicsAPI::DepthStencilTarget* gbufferDepthStencilTarget = nullptr;
			GraphicsAPI::Framebuffer* litHdrFramebuffer = nullptr;
			GraphicsAPI::Framebuffer* lightingFramebuffer = nullptr;
			GraphicsAPI::RenderTarget* litHdrRenderTarget = nullptr;
			GraphicsAPI::RenderTarget* ssrRenderTarget = nullptr;
			GraphicsAPI::RenderTarget* gbufferAlbedoRenderTarget = nullptr;
			GraphicsAPI::RenderTarget* gbufferNormalRenderTarget = nullptr;
			GraphicsAPI::RenderTarget* gbufferSpecularRoughnessRenderTarget = nullptr;

			GraphicsAPI::Framebuffer* ambientOcclusionFramebuffer = nullptr;
			GraphicsAPI::RenderTarget* ambientOcclusionRenderTarget = nullptr;

			GraphicsAPI::UniformBuffer* globalUniformBufferObject = nullptr;
			GraphicsAPI::UniformBuffer* debugUniformBufferObject = nullptr;
			GraphicsAPI::UniformBuffer* tonemapPostProcessingUniformBufferObject = nullptr;

			GraphicsAPI::DescriptorSet* ssrDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* tonemapDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* lightingDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* debugDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* engineDescriptorSet = nullptr;

			GraphicsAPI::DescriptorSet* dofSourceDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* dofNearBlurDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* dofFarBlurDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* dofCombineDescriptorSet = nullptr;

			GraphicsAPI::DescriptorSet* ambientOcclusionDescriptorSet = nullptr;

			GraphicsAPI::RenderTarget* nearDofRenderTarget = nullptr;
			GraphicsAPI::RenderTarget* farDofRenderTarget = nullptr;
			GraphicsAPI::RenderTarget* nearBlurredDofRenderTarget = nullptr;
			GraphicsAPI::RenderTarget* farBlurredDofRenderTarget = nullptr;

			GraphicsAPI::Framebuffer* dofSeparationFramebuffer = nullptr;
			GraphicsAPI::Framebuffer* dofNearBlurFramebuffer = nullptr;
			GraphicsAPI::Framebuffer* dofFarBlurFramebuffer = nullptr;
			GraphicsAPI::Framebuffer* dofCombinationFramebuffer = nullptr;

			std::vector<GraphicsAPI::RenderTarget*> bloomRenderTargets;
			std::vector<GraphicsAPI::DescriptorSet*> bloomDescriptorSets;
		};

		std::map<Uuid, GraphicsAPI::GraphicsPipeline*> graphicsPipelineMap;

		void CreatePipelines();
		void CreateBloomUniformBuffers();
		void UpdateBloomDescriptorSet(DeferredRendererImageSet& imageSet);
		void CreateDepthOfFieldRenderTargetsAndDescriptorSets(DeferredRendererImageSet& imageSet, size_t imageSetIndex);
		void CreateSsrRenderTargetsAndDescriptorSets(DeferredRendererImageSet& imageSet, size_t imageSetIndex);
		void CreateBloomRenderTargetsAndDescriptorSets(DeferredRendererImageSet& imageSet, size_t imageSetIndex);
		void RenderDepthOfField(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer);
		void RenderSsr(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer);
		void RenderBloom(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer);
		void UpdateBloomUBO();

		void RenderSsao(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* commandBuffer);
		void RenderShadowMaps(GraphicsAPI::CommandBuffer* commandBuffer, entt::registry& registry);
		void RenderLights(uint32_t imageIndex, GraphicsAPI::CommandBuffer* currentCommandBuffer, entt::registry& registry);
		void PostProcess(uint32_t imageIndex, GraphicsAPI::Framebuffer* framebuffer, GraphicsAPI::CommandBuffer* currentCommandBuffer);
		void Debug(uint32_t imageIndex, GraphicsAPI::Framebuffer* outputFramebuffer, GraphicsAPI::CommandBuffer* commandBuffer);

		void CreateDepthOfFieldResources();
		void CreateBloomResources();
		void CreateSSRResources();
		void CreateSsaoKernelAndNoise();
		void CleanupPipelines();
		void CreateDescriptorSetLayouts();
		void CreateGbufferFramebuffer();
		void CreateLitHDRFramebuffer();
		void CreateDescriptorSets(DeferredRendererImageSet& imageSet);
		void UpdateDescriptorSets(DeferredRendererImageSet& imageSet);
		void CreateUniformBuffers();
		void CreateVertexAndIndexBuffersAndLayouts();

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

		PostProcessUbo postProcessUboData;

		struct DebugUboData {
			uint16_t renderMode;
		};

		DebugUboData debugUboData;

		uint32_t framebufferWidth = 0u;
		uint32_t framebufferHeight = 0u;
		uint32_t renderWidth = 0u;
		uint32_t renderHeight = 0u;

		size_t bloomStoredMipLevelCount = 0;
		size_t bloomMipLevelCount = 0;
		Grindstone::GraphicsAPI::Texture* brdfLut = nullptr;

		std::vector<DeferredRendererImageSet> deferredRendererImageSets;
		std::vector<GraphicsAPI::UniformBuffer*> bloomUniformBuffers;
		size_t bloomFirstUpsampleIndex = 0;

		GraphicsAPI::RenderPass* dofSeparationRenderPass = nullptr;
		GraphicsAPI::RenderPass* dofBlurAndCombinationRenderPass = nullptr;

		GraphicsAPI::RenderPass* lightingRenderPass = nullptr;
		GraphicsAPI::RenderPass* forwardLitRenderPass = nullptr;
		GraphicsAPI::RenderPass* ssaoRenderPass = nullptr;
		GraphicsAPI::UniformBuffer* ssaoUniformBuffer = nullptr;
		GraphicsAPI::Texture* ssaoNoiseTexture = nullptr;
		GraphicsAPI::DescriptorSetLayout* ambientOcclusionDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* ssaoInputDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSet* ssaoInputDescriptorSet = nullptr;

		GraphicsAPI::DescriptorSetLayout* environmentMapDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSet* environmentMapDescriptorSet = nullptr;

		GraphicsAPI::VertexBufferLayout vertexLightPositionLayout{};

		GraphicsAPI::DescriptorSetLayout* bloomDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* ssrDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* tonemapDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* debugDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* lightingDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* lightingUBODescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* shadowMappedLightDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* lightingWithShadowUBODescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* engineDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* shadowMapDescriptorSetLayout = nullptr;

		GraphicsAPI::DescriptorSet* shadowMapDescriptorSet = nullptr;

		GraphicsAPI::RenderPass* shadowMapRenderPass = nullptr;
		GraphicsAPI::RenderPass* targetRenderPass = nullptr;

		GraphicsAPI::VertexBuffer* vertexBuffer;
		GraphicsAPI::IndexBuffer* indexBuffer;
		GraphicsAPI::VertexArrayObject* planePostProcessVao = nullptr;

		GraphicsAPI::GraphicsPipeline* ssaoPipeline = nullptr;
		GraphicsAPI::GraphicsPipeline* imageBasedLightingPipeline = nullptr;
		GraphicsAPI::GraphicsPipeline* spotLightPipeline = nullptr;
		GraphicsAPI::GraphicsPipeline* pointLightPipeline = nullptr;
		GraphicsAPI::GraphicsPipeline* directionalLightPipeline = nullptr;
		GraphicsAPI::GraphicsPipeline* tonemapPipeline = nullptr;
		GraphicsAPI::GraphicsPipeline* dofSeparationPipeline = nullptr;
		GraphicsAPI::GraphicsPipeline* dofBlurPipeline = nullptr;
		GraphicsAPI::GraphicsPipeline* dofCombinationPipeline = nullptr;
		GraphicsAPI::GraphicsPipeline* shadowMappingPipeline = nullptr;
		GraphicsAPI::GraphicsPipeline* debugPipeline = nullptr;

		GraphicsAPI::DescriptorSetLayout* dofSourceDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* dofBlurDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* dofCombinationDescriptorSetLayout = nullptr;

		GraphicsAPI::ComputePipeline* ssrPipeline = nullptr;
		GraphicsAPI::ComputePipeline* bloomPipeline = nullptr;

		static std::array<Grindstone::BaseRenderer::RenderMode, static_cast<uint16_t>(DeferredRenderMode::Count)> renderModes;

		// Used to check when environment map changes, so we can update it
		Uuid currentEnvironmentMapUuid;

		DeferredRenderMode renderMode;
	};
}
