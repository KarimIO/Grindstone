#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Common/Math.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include "GizmoRenderer.hpp"

struct EngineUboStruct {
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 inverseProjectionMatrix;
	glm::mat4 inverseViewMatrix;
	glm::vec3 eyePos;
	float alignmentBufferForPreviousVec3;
	glm::vec2 framebufferResolution;
	glm::vec2 renderResolution;
	glm::vec2 renderScale;
	float time;
};

namespace Grindstone {
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
			EditorCamera();
			~EditorCamera();
			void CaptureMousePick(Grindstone::Math::Int2 coordinates);
			uint64_t GetRenderOutput();
			void Render(GraphicsAPI::CommandBuffer* commandBuffer);
			void RenderPlayModeCamera(GraphicsAPI::CommandBuffer* commandBuffer);
			void OffsetRotation(float pitch, float yaw);
			void OffsetPosition(glm::vec3 offset);
			void SetPosition(glm::vec3 newPosition);
			void ResizeViewport(uint32_t width, uint32_t height);
			void UpdateProjectionMatrix();
			void UpdateViewMatrix();
			glm::mat4& GetProjectionMatrix();
			glm::mat4& GetViewMatrix();

			glm::vec3 GetPosition() const;
			glm::vec3 GetForward() const;
			glm::vec3 GetRight() const;
			glm::vec3 GetUp() const;

			bool isGridEnabled = true;
			bool isBoundingSphereGizmoEnabled = false;
			bool isBoundingBoxGizmoEnabled = false;
			uint32_t renderMode = 0;
			bool isColliderGizmoEnabled = true;
			bool mousePickedThisFrame = false;
			Grindstone::Math::Int2 mousePickCoordinates;
		private:
			GraphicsAPI::Buffer* gpuGlobalUniformBufferObject = nullptr;

			Grindstone::GraphicsAPI::DescriptorSetLayout* globalDescriptorSetLayout;
			Grindstone::GraphicsAPI::Buffer* globalStagingUniformBufferObject;
			std::array<Grindstone::GraphicsAPI::Buffer*, 3> globalUniformBufferObject;
			std::array<Grindstone::GraphicsAPI::DescriptorSet*, 3> globalDescriptorSet;

			GizmoRenderer gizmoRenderer;
			std::array<GraphicsAPI::Image*, 3> renderTarget;
			std::array<GraphicsAPI::Image*, 3> depthTarget;
			GraphicsAPI::Sampler* sampler = nullptr;
			GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;
			std::array<GraphicsAPI::DescriptorSet*, 3> descriptorSet;

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
