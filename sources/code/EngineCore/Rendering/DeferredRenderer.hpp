#pragma once

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
		virtual void Render(
			entt::registry& registry,
			glm::mat4 projectionMatrix,
			glm::mat4 viewMatrix,
			glm::vec3 eyePos,
			GraphicsAPI::Framebuffer* outputFramebuffer
		) override;
	private:
		void RenderLights(entt::registry& registry);
		void PostProcess(GraphicsAPI::Framebuffer* outputFramebuffer);

		void CreateDeferredRendererInstanceObjects();
		void CreateDeferredRendererStaticObjects();

		GraphicsAPI::UniformBufferBinding* globalUniformBufferBinding = nullptr;
		GraphicsAPI::UniformBuffer* globalUniformBufferObject = nullptr;
		GraphicsAPI::UniformBufferBinding* lightUniformBufferBinding = nullptr;
		GraphicsAPI::UniformBuffer* lightUniformBufferObject = nullptr;
		GraphicsAPI::Framebuffer* gbuffer = nullptr;
		GraphicsAPI::Framebuffer* litHdrFramebuffer = nullptr;
		GraphicsAPI::RenderTarget* renderTargets = nullptr;
		GraphicsAPI::RenderTarget* litHdrImages = nullptr;
		GraphicsAPI::DepthTarget* depthTarget = nullptr;

		GraphicsAPI::VertexArrayObject* planePostProcessVao = nullptr;
		GraphicsAPI::Pipeline* lightPipeline = nullptr;
		GraphicsAPI::Pipeline* tonemapPipeline = nullptr;
	};
}
