#pragma once

#include <vector>
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
			std::vector<GraphicsAPI::RenderTarget*> gbufferRenderTargets;
			GraphicsAPI::DepthTarget* gbufferDepthTarget = nullptr;
			GraphicsAPI::Framebuffer* litHdrFramebuffer = nullptr;
			GraphicsAPI::RenderTarget* litHdrRenderTarget = nullptr;
			GraphicsAPI::DepthTarget* litHdrDepthTarget = nullptr;

			GraphicsAPI::UniformBuffer* globalUniformBufferObject = nullptr;

			GraphicsAPI::DescriptorSet* tonemapDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* lightingDescriptorSet = nullptr;
			GraphicsAPI::DescriptorSet* engineDescriptorSet = nullptr;

			std::vector<GraphicsAPI::RenderTarget*> bloomRenderTargets;
			std::vector<GraphicsAPI::DescriptorSet*> bloomDescriptorSets;
		};

		std::map<Uuid, GraphicsAPI::GraphicsPipeline*> graphicsPipelineMap;

		void CreatePipelines();
		void CreateBloomUniformBuffers();
		void CreateBloomRenderTargetsAndDescriptorSets(DeferredRendererImageSet& imageSet, size_t imageSetIndex);
		void RenderBloom(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer);

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

		void CreateBloomResources();
		void CreateSsaoKernelAndNoise();
		void CleanupPipelines();
		void CreateDescriptorSetLayouts();
		void CreateGbufferFramebuffer();
		void CreateLitHDRFramebuffer();
		void CreateDescriptorSets(DeferredRendererImageSet& imageSet);
		void UpdateDescriptorSets(DeferredRendererImageSet& imageSet);
		void CreateUniformBuffers();
		void CreateVertexAndIndexBuffersAndLayouts();

		uint32_t width = 800;
		uint32_t height = 600;
		size_t bloomMipLevelCount = 0;
		Grindstone::GraphicsAPI::Texture* brdfLut = nullptr;

		std::vector<DeferredRendererImageSet> deferredRendererImageSets;
		std::vector<GraphicsAPI::UniformBuffer*> bloomUniformBuffers;

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
		GraphicsAPI::GraphicsPipeline* shadowMappingPipeline = nullptr;

		GraphicsAPI::ComputePipeline* bloomPipeline = nullptr;

		// Used to check when environment map changes, so we can update it
		Uuid currentEnvironmentMapUuid;
	};
}
