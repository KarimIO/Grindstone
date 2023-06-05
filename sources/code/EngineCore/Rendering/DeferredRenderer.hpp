#pragma once

#include <vector>
#include "BaseRenderer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class UniformBufferBinding;
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
		void RenderLights(entt::registry& registry);
		void PostProcess(GraphicsAPI::Framebuffer* outputFramebuffer);

		void CreateDeferredRendererInstanceObjects();
		void CreateDeferredRendererStaticObjects();

		uint32_t width = 800;
		uint32_t height = 600;

		std::vector<GraphicsAPI::RenderTarget*> gbufferRenderTargets;

		GraphicsAPI::UniformBufferBinding* globalUniformBufferBinding = nullptr;
		GraphicsAPI::UniformBuffer* globalUniformBufferObject = nullptr;
		GraphicsAPI::UniformBufferBinding* lightUniformBufferBinding = nullptr;
		GraphicsAPI::UniformBuffer* lightUniformBufferObject = nullptr;
		GraphicsAPI::Framebuffer* gbuffer = nullptr;
		GraphicsAPI::Framebuffer* litHdrFramebuffer = nullptr;
		GraphicsAPI::RenderTarget* litHdrRenderTarget = nullptr;
		GraphicsAPI::DepthTarget* gbufferDepthTarget = nullptr;
		GraphicsAPI::DepthTarget* litHdrDepthTarget = nullptr;
		GraphicsAPI::RenderPass* mainRenderPass = nullptr;

		GraphicsAPI::VertexArrayObject* planePostProcessVao = nullptr;
		GraphicsAPI::Pipeline* lightPipeline = nullptr;
		GraphicsAPI::Pipeline* tonemapPipeline = nullptr;
	};
}
