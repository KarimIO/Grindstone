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
	};

	class DeferredRenderer : public BaseRenderer {
	public:
		DeferredRenderer();
		virtual ~DeferredRenderer();
		virtual bool OnWindowResize(Events::BaseEvent*) override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void Render(
			entt::registry& registry,
			glm::mat4 projectionMatrix,
			glm::mat4 viewMatrix,
			glm::vec3 eyePos,
			GraphicsAPI::Framebuffer* outputFramebuffer
		) override;
		static GraphicsAPI::RenderPass* gbufferRenderPass;
	private:
		void RenderCommandBuffer(
			entt::registry& registry,
			GraphicsAPI::Framebuffer* outputFramebuffer
		);
		void RenderImmediate(
			entt::registry& registry,
			GraphicsAPI::Framebuffer* outputFramebuffer
		);
		void RenderLightsCommandBuffer(GraphicsAPI::CommandBuffer* currentCommandBuffer, entt::registry& registry);
		void RenderLightsImmediate(entt::registry& registry);
		void PostProcessCommandBuffer(GraphicsAPI::RenderPass* renderPass, GraphicsAPI::Framebuffer* framebuffer, GraphicsAPI::CommandBuffer* currentCommandBuffer);
		void PostProcessImmediate(GraphicsAPI::Framebuffer* outputFramebuffer);

		void CreateCommandBuffers();
		void CreateGbufferFramebuffer();
		void CreateLitHDRFramebuffer();
		void CreatePipelines();
		void CreateDescriptorSetLayouts();
		void CreateDescriptorSets();
		void CreateUniformBuffers();
		void CreateVertexAndIndexBuffersAndLayouts();

		uint32_t width = 800;
		uint32_t height = 600;

		std::vector<GraphicsAPI::RenderTarget*> gbufferRenderTargets;

		GraphicsAPI::VertexBufferLayout vertexLightPositionLayout{};

		GraphicsAPI::DescriptorSetLayout* tonemapDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* lightingDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSetLayout* engineDescriptorSetLayout = nullptr;
		GraphicsAPI::DescriptorSet* tonemapDescriptorSet = nullptr;
		GraphicsAPI::DescriptorSet* lightingDescriptorSet = nullptr;
		GraphicsAPI::DescriptorSet* engineDescriptorSet = nullptr;

		GraphicsAPI::UniformBuffer* globalUniformBufferObject = nullptr;
		GraphicsAPI::UniformBuffer* lightUniformBufferObject = nullptr;
		GraphicsAPI::Framebuffer* gbuffer = nullptr;
		GraphicsAPI::Framebuffer* litHdrFramebuffer = nullptr;
		GraphicsAPI::RenderTarget* litHdrRenderTarget = nullptr;
		GraphicsAPI::DepthTarget* gbufferDepthTarget = nullptr;
		GraphicsAPI::DepthTarget* litHdrDepthTarget = nullptr;
		GraphicsAPI::RenderPass* mainRenderPass = nullptr;

		GraphicsAPI::VertexBuffer* vertexBuffer;
		GraphicsAPI::IndexBuffer* indexBuffer;
		GraphicsAPI::VertexArrayObject* planePostProcessVao = nullptr;

		GraphicsAPI::Pipeline* pointLightPipeline = nullptr;
		GraphicsAPI::Pipeline* tonemapPipeline = nullptr;

		std::vector<GraphicsAPI::CommandBuffer*> commandBuffers;
	};
}
