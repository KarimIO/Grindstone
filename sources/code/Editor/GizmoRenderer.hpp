#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace Grindstone::GraphicsAPI {
	class CommandBuffer;
	class UniformBuffer;
	class DescriptorSet;
	class DescriptorSetLayout;
	class GraphicsPipeline;
	class VertexBuffer;
	class IndexBuffer;
	class VertexArrayObject;
	class RenderPass;
}

namespace Grindstone::Editor {
	class GizmoRenderer {
	public:
		void Initialize(class Grindstone::GraphicsAPI::RenderPass* renderPass);
		void SubmitCubeGizmo(const glm::mat4& transform, glm::vec3 size, glm::vec4 color = glm::vec4(1.0f));
		void SubmitCapsuleGizmo(const glm::mat4& transform, float height, float radius, glm::vec4 color = glm::vec4(1.0f));
		void SubmitPlaneGizmo(const glm::mat4& transform, glm::vec3 normal, float positionAlongNormal, glm::vec4 color = glm::vec4(1.0f));
		void SubmitSphereGizmo(const glm::mat4& transform, float radius, glm::vec4 color = glm::vec4(1.0f));
		void Render(Grindstone::GraphicsAPI::CommandBuffer* commandBuffer, glm::mat4 projView);
	protected:
		Grindstone::GraphicsAPI::UniformBuffer* gizmoUniformBuffer = nullptr;
		Grindstone::GraphicsAPI::DescriptorSet* gizmoDescriptorSet = nullptr;
		Grindstone::GraphicsAPI::DescriptorSetLayout* gizmoDescriptorSetLayout = nullptr;
		Grindstone::GraphicsAPI::GraphicsPipeline* gizmoPipeline = nullptr;

		Grindstone::GraphicsAPI::VertexBuffer* gizmoShapesVertexBuffer = nullptr;
		Grindstone::GraphicsAPI::IndexBuffer* gizmoShapesIndexBuffer = nullptr;
		Grindstone::GraphicsAPI::VertexArrayObject* gizmoShapesVao = nullptr;

		enum class ShapeType {
			Undefined,
			Cube,
			Circle,
			Sphere,
			Cone,
			Plane,
			Capsule,
			Cyclinder,
		};

		struct ShapeMetaData {
			uint32_t firstIndex;
			uint32_t indexCount;
			uint32_t vertexOffset;
		};

		std::array<ShapeMetaData, 8> shapeMetaData;

		struct GizmoUniformBuffer {
			glm::mat4 transform;
			glm::vec4 color;
		};

		std::vector<GizmoUniformBuffer> dataBuffer;
		std::vector<ShapeType> drawShapes;
		size_t drawCount = 0;

		inline void AppendData(ShapeType shapeType, uint32_t& currentVertexOffset, uint32_t& currentIndexOffset, uint32_t vertexCount, uint32_t indexCount);
	};
}
