#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "GizmoRenderer.hpp"

namespace Grindstone {
	class BaseRenderer;
	struct TransformComponent;
	struct CameraComponent;

	namespace GraphicsAPI {
		class Framebuffer;
		class CommandBuffer;
		class DescriptorSet;
		class DescriptorSetLayout;
		class RenderTarget;
		class RenderPass;
	}

	namespace Editor {
		class EditorCamera {
		public:
			EditorCamera();
			~EditorCamera();
			uint64_t GetRenderOutput();
			void Render(GraphicsAPI::CommandBuffer* commandBuffer);
			void RenderPlayModeCamera(GraphicsAPI::CommandBuffer* commandBuffer);
			void OffsetRotation(float pitch, float yaw);
			void OffsetPosition(float x, float y, float z);
			void ResizeViewport(uint32_t width, uint32_t height);
			void UpdateProjectionMatrix();
			void UpdateViewMatrix();
			glm::mat4& GetProjectionMatrix();
			glm::mat4& GetViewMatrix();
			BaseRenderer* GetRenderer() const;
		private:
			glm::vec3 GetForward() const;
			glm::vec3 GetRight() const;
			glm::vec3 GetUp() const;

			GizmoRenderer gizmoRenderer;
			GraphicsAPI::RenderTarget* renderTarget = nullptr;
			GraphicsAPI::RenderPass* renderPass = nullptr;
			GraphicsAPI::RenderPass* gizmoRenderPass = nullptr;
			GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;
			GraphicsAPI::Framebuffer* framebuffer = nullptr;
			GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
			BaseRenderer* renderer = nullptr;
			glm::mat4 projection;
			glm::mat4 view;
			glm::vec3 position = glm::vec3();
			glm::vec3 eulerAngles = glm::vec3();
			glm::quat rotation = glm::quat();
			uint32_t width = 800;
			uint32_t height = 600;
			float fieldOfView = glm::radians(80.0f);
			float nearPlaneDistance = 0.1f;
			float farPlaneDistance = 150.f;
		};
	}
}
