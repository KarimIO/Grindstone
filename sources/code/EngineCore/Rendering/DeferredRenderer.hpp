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
		class DepthTarget;
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

		static GraphicsAPI::RenderPass* gbufferRenderPass;
		static GraphicsAPI::RenderPass* mainRenderPass;
	private:
		struct DeferredRendererImageSet {
			GraphicsAPI::Framebuffer* gbuffer = nullptr;
			GraphicsAPI::DepthTarget* gbufferDepthTarget = nullptr;
			GraphicsAPI::Framebuffer* litHdrFramebuffer = nullptr;
			GraphicsAPI::Framebuffer* lightingFramebuffer = nullptr;
			GraphicsAPI::RenderTarget* litHdrRenderTarget = nullptr;
			GraphicsAPI::DepthTarget* litHdrDepthTarget = nullptr;
			GraphicsAPI::RenderTarget* ssrRenderTarget = nullptr;
			GraphicsAPI::RenderTarget* gbufferAlbedoRenderTarget = nullptr;
			GraphicsAPI::RenderTarget* gbufferNormalRenderTarget = nullptr;
			GraphicsAPI::RenderTarget* gbufferSpecularRoughnessRenderTarget = nullptr;

			GraphicsAPI::UniformBuffer* globalUniformBufferObject = nullptr;
			GraphicsAPI::UniformBuffer* tonemapPostProcessingUniformBufferObject = nullptr;
			GraphicsAPI::UniformBuffer* ssrUbo = nullptr;

			GraphicsAPI::DescriptorSet* ssrDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* tonemapDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* lightingDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* engineDescriptorSet = nullptr;

			GraphicsAPI::DescriptorSet* dofSeparationDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* dofBloomDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* dofCombineDescriptorSet = nullptr;

			GraphicsAPI::RenderTarget* nearDofRenderTarget = nullptr;
			GraphicsAPI::RenderTarget* farDofRenderTarget = nullptr;

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

		void RenderCommandBuffer(
			GraphicsAPI::CommandBuffer* commandBuffer,
			entt::registry& registry,
			glm::mat4 projectionMatrix,
			glm::mat4 viewMatrix,
			glm::vec3 eyePos,
			GraphicsAPI::Framebuffer* outputFramebuffer
		);
		void RenderImmediate(
			entt::registry& registry,
			glm::mat4 projectionMatrix,
			glm::mat4 viewMatrix,
			glm::vec3 eyePos,
			GraphicsAPI::Framebuffer* outputFramebuffer
		);

		void RenderSsao(uint32_t imageIndex, GraphicsAPI::CommandBuffer* commandBuffer);
		void RenderShadowMaps(GraphicsAPI::CommandBuffer* commandBuffer, entt::registry& registry);
		void RenderLightsCommandBuffer(uint32_t imageIndex, GraphicsAPI::CommandBuffer* currentCommandBuffer, entt::registry& registry);
		void RenderLightsImmediate(entt::registry& registry);
		void PostProcessCommandBuffer(uint32_t imageIndex, GraphicsAPI::Framebuffer* framebuffer, GraphicsAPI::CommandBuffer* currentCommandBuffer);
		void PostProcessImmediate(GraphicsAPI::Framebuffer* outputFramebuffer);

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
			float grainAmount = 0.007f;
			float grainPixelSize = 1.0f;
			glm::vec2 chromaticDistortionRedOffset = glm::vec2(0.0045f, 0.0045f);
			glm::vec2 chromaticDistortionGreenOffset = glm::vec2(0.003f, 0.003f);
			glm::vec2 chromaticDistortionBlueOffset = glm::vec2(-0.003f, -0.003f);
			float paniniDistortionStrength = 1.00f;
			bool isAnimated = true;
		};

		PostProcessUbo postProcessUboData;

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
		GraphicsAPI::Framebuffer* ssaoFramebuffer = nullptr;
		GraphicsAPI::RenderTarget* ssaoRenderTarget = nullptr;
		GraphicsAPI::UniformBuffer* ssaoUniformBuffer = nullptr;
		GraphicsAPI::Texture* ssaoNoiseTexture = nullptr;
		GraphicsAPI::DescriptorSetLayout* ssaoInputDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSet* ssaoInputDescriptorSet = nullptr;
		GraphicsAPI::DescriptorSetLayout* ssaoDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSet* ssaoDescriptorSet = nullptr;

		GraphicsAPI::DescriptorSetLayout* environmentMapDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSet* environmentMapDescriptorSet = nullptr;

		GraphicsAPI::VertexBufferLayout vertexLightPositionLayout{};

		GraphicsAPI::DescriptorSetLayout* bloomDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* ssrDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* tonemapDescriptorSetLayout = nullptr;
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

		GraphicsAPI::ComputePipeline* ssrPipeline = nullptr;
		GraphicsAPI::ComputePipeline* bloomPipeline = nullptr;

		// Used to check when environment map changes, so we can update it
		Uuid currentEnvironmentMapUuid;
	};
}
