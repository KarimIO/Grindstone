#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <Common/Rendering/GeometryRenderingStats.hpp>
#include <Common/Graphics/CommandBuffer.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class RenderPass;
		class Framebuffer;
		class CommandBuffer;
	}

	namespace Events {
		struct BaseEvent;
	}

	class BaseRenderer {
	public:
		struct RenderMode {
			const char* name = nullptr;
		};

		virtual ~BaseRenderer() {};
		virtual bool OnWindowResize(Events::BaseEvent*) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Render(
			GraphicsAPI::CommandBuffer* commandBuffer,
			entt::registry& registry,
			glm::mat4 projectionMatrix,
			glm::mat4 viewMatrix,
			glm::vec3 eyePos,
			Grindstone::GraphicsAPI::RenderAttachment& outAttachment // If nullptr, use default framebuffer
		) = 0;
		virtual uint16_t GetRenderModeCount() const = 0;
		virtual const RenderMode* GetRenderModes() const = 0;
		virtual void SetRenderMode(uint16_t mode) = 0;
		virtual std::vector<Grindstone::Rendering::GeometryRenderStats> GetRenderingStats() = 0;
	};

	class BaseRendererFactory {
	public:
		virtual Grindstone::BaseRenderer* CreateRenderer(GraphicsAPI::RenderPass* targetRenderPass) = 0;
		virtual ~BaseRendererFactory() {}
	};
}
