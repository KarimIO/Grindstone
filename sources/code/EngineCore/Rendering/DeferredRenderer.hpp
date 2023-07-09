#pragma once

#include <vector>
#include "BaseRenderer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class UniformBuffer;
		class Framebuffer;
		class RenderTarget;
		class DepthTarget;
		class VertexArrayObject;
		class Pipeline;
		class CommandBuffer;
	};

	class DeferredRenderer : public BaseRenderer {
	public:
		DeferredRenderer();
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
		};

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

		void RenderShadowMaps(GraphicsAPI::CommandBuffer* commandBuffer, entt::registry& registry);
		void RenderLightsCommandBuffer(uint32_t imageIndex, GraphicsAPI::CommandBuffer* currentCommandBuffer, entt::registry& registry);
		void RenderLightsImmediate(entt::registry& registry);
		void PostProcessCommandBuffer(uint32_t imageIndex, GraphicsAPI::Framebuffer* framebuffer, GraphicsAPI::CommandBuffer* currentCommandBuffer);
		void PostProcessImmediate(GraphicsAPI::Framebuffer* outputFramebuffer);

		void CleanupPipelines();
		void CreatePipelines();
		void CreateDescriptorSetLayouts();
		void CreateCommandBuffers();
		void CreateGbufferFramebuffer();
		void CreateLitHDRFramebuffer();
		void CreateDescriptorSets(DeferredRendererImageSet& imageSet);
		void UpdateDescriptorSets(DeferredRendererImageSet& imageSet);
		void CreateUniformBuffers();
		void CreateVertexAndIndexBuffersAndLayouts();

		uint32_t width = 800;
		uint32_t height = 600;

		std::vector<DeferredRendererImageSet> deferredRendererImageSets;

		GraphicsAPI::VertexBufferLayout vertexLightPositionLayout{};

		GraphicsAPI::DescriptorSetLayout* tonemapDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* lightingDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* lightingUBODescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* shadowMappedLightDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* lightingWithShadowUBODescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* engineDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* shadowMapDescriptorSetLayout = nullptr;

		GraphicsAPI::DescriptorSet* shadowMapDescriptorSet = nullptr;

		GraphicsAPI::RenderPass* mainRenderPass = nullptr;
		GraphicsAPI::RenderPass* shadowMapRenderPass = nullptr;

		GraphicsAPI::VertexBuffer* vertexBuffer;
		GraphicsAPI::IndexBuffer* indexBuffer;
		GraphicsAPI::VertexArrayObject* planePostProcessVao = nullptr;

		GraphicsAPI::Pipeline* spotLightPipeline = nullptr;
		GraphicsAPI::Pipeline* pointLightPipeline = nullptr;
		GraphicsAPI::Pipeline* directionalLightPipeline = nullptr;
		GraphicsAPI::Pipeline* tonemapPipeline = nullptr;
		GraphicsAPI::Pipeline* shadowMappingPipeline = nullptr;
	};
}
