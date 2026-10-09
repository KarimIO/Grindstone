#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <functional>

#include <Common/Blackboard/Blackboard.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <Editor/Selection.hpp>

namespace Grindstone::GraphicsAPI {
	class CommandBuffer;
	class DescriptorSet;
	class DescriptorSetLayout;
	class GraphicsPipeline;
	class Buffer;
	class VertexArrayObject;
	class RenderPass;
}

namespace Grindstone::Editor {
	class GizmoRenderer;
	using GizmoRendererCallback = std::function<void(Grindstone::Editor::GizmoRenderer& gizmoRenderer, Grindstone::Blackboard& blackboard, const Grindstone::Editor::Selection& selection)>;

	class GizmoRenderer {
	public:
		void Initialize();
		virtual void SubmitCubeGizmo(const glm::mat4& transform, glm::vec3 size, glm::vec4 color = glm::vec4(1.0f));
		virtual void SubmitCapsuleGizmo(const glm::mat4& transform, float height, float radius, glm::vec4 color = glm::vec4(1.0f));
		virtual void SubmitPlaneGizmo(const glm::mat4& transform, glm::vec3 normal, float positionAlongNormal, glm::vec4 color = glm::vec4(1.0f));
		virtual void SubmitSphereGizmo(const glm::mat4& transform, float radius, glm::vec4 color = glm::vec4(1.0f));
		virtual void RegisterGizmoCallback(Grindstone::HashedString name, GizmoRendererCallback callback);
		virtual void UnregisterGizmoCallback(Grindstone::HashedString name);

		void Render(Grindstone::GraphicsAPI::CommandBuffer* commandBuffer, glm::mat4 projView);
		void IterateOnGizmoCallbacks(Grindstone::Blackboard& blackboard, const Grindstone::Editor::Selection& selection);
	protected:
		struct GizmoRendererCallbackEntry {
			Grindstone::HashedString name;
			GizmoRendererCallback callback;
		};
		std::vector<GizmoRendererCallbackEntry> gizmoCallbacks;
		std::array<Grindstone::GraphicsAPI::Buffer*, 3> gizmoUniformBuffers = {};
		std::array<Grindstone::GraphicsAPI::DescriptorSet*, 3> gizmoDescriptorSets = {};
		Grindstone::GraphicsAPI::DescriptorSetLayout* gizmoDescriptorSetLayout = nullptr;
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> gizmoPipelineSet;
		GraphicsAPI::VertexInputLayout gizmoVertexLayout;

		Grindstone::GraphicsAPI::Buffer* gizmoShapesVertexBuffer = nullptr;
		Grindstone::GraphicsAPI::Buffer* gizmoShapesIndexBuffer = nullptr;
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
			uint32_t firstOutlineIndex;
			uint32_t outlineIndexCount;
			uint32_t firstFilledIndex;
			uint32_t filledIndexCount;
			uint32_t vertexOffset;
		};

		std::array<ShapeMetaData, 8> shapeMetaData;

		struct GizmoUniformBuffer {
			glm::mat4 transform;
			glm::vec4 outlineColor;
			glm::vec4 fillColor;
		};

		std::vector<GizmoUniformBuffer> dataBuffer;
		std::vector<ShapeType> drawShapes;
		size_t drawCount = 0;

		inline void AppendData(
			ShapeType shapeType,
			uint32_t& currentVertexOffset, uint32_t& currentOutlineIndexOffset, uint32_t& currentFilledIndexOffset,
			uint32_t vertexCount, uint32_t outlineIndexCount, uint32_t filledIndexCount
		);
	};
}
