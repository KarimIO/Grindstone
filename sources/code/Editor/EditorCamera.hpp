#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "GridRenderer.hpp"
#include "GizmoRenderer.hpp"

namespace Grindstone {
	class BaseRenderer;
	struct TransformComponent;
	struct CameraComponent;

	namespace GraphicsAPI {
		class Framebuffer;
		class Image;
		class Sampler;
		class CommandBuffer;
		class DescriptorSet;
		class DescriptorSetLayout;
		class RenderPass;
	}

	namespace Editor {
		class EditorCamera {
		public:
			static void SetupRenderPasses();
			EditorCamera();
			~EditorCamera();
			void CaptureMousePick(GraphicsAPI::CommandBuffer* commandBuffer, int x, int y);
			bool HasQueuedMousePick() const;
			uint32_t GetMousePickedEntity(GraphicsAPI::CommandBuffer* commandBuffer);
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

			bool isGridEnabled = true;
			bool isBoundingSphereGizmoEnabled = false;
			bool isBoundingBoxGizmoEnabled = false;
			bool isColliderGizmoEnabled = true;
		private:
			glm::vec3 GetForward() const;
			glm::vec3 GetRight() const;
			glm::vec3 GetUp() const;

			GizmoRenderer gizmoRenderer;
			GridRenderer gridRenderer;
			GraphicsAPI::Image* renderTarget = nullptr;
			GraphicsAPI::Image* depthTarget = nullptr;
			GraphicsAPI::Sampler* sampler = nullptr;
			GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;
			GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
			GraphicsAPI::Framebuffer* framebuffer = nullptr;

			GraphicsAPI::DescriptorSetLayout* mousePickDescriptorSetLayout = nullptr;
			std::array<GraphicsAPI::Image*, 3> mousePickRenderTarget{};
			std::array<GraphicsAPI::Framebuffer*, 3> mousePickFramebuffer{};
			std::array<GraphicsAPI::DescriptorSet*, 3> mousePickDescriptorSet{};
			std::array<GraphicsAPI::Buffer*, 3> mousePickMatrixBuffer{};
			std::array<GraphicsAPI::Buffer*, 3> mousePickResponseBuffer{};

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
			bool hasQueuedMousePick = false;
		};
	}
}
