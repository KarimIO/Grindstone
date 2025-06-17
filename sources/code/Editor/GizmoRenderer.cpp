#include <glm/gtx/transform.hpp>

#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/VertexArrayObject.hpp>
#include <Common/Graphics/CommandBuffer.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <Common/Graphics/DescriptorSet.hpp>
#include <Common/Graphics/DescriptorSetLayout.hpp>
#include <Common/Graphics/GraphicsPipeline.hpp>
#include <Common/Graphics/Formats.hpp>
#include <Editor/EditorManager.hpp>

#include "GizmoRenderer.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor;

static size_t maxObjects = 500;

inline void GizmoRenderer::AppendData(
	ShapeType shapeType,
	uint32_t& currentVertexOffset, uint32_t& currentIndexOffset,
	uint32_t vertexCount, uint32_t indexCount
) {
	indexCount *= 2;

	shapeMetaData[static_cast<size_t>(shapeType)] = { currentIndexOffset, indexCount, currentVertexOffset };

	currentVertexOffset += vertexCount * 3;
	currentIndexOffset += indexCount;
}

void GizmoRenderer::Initialize(GraphicsAPI::RenderPass* renderPass) {
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	GraphicsAPI::VertexInputLayoutBuilder layoutBuilder;
	gizmoVertexLayout = layoutBuilder
		.AddBinding(
			{ 0, 0, GraphicsAPI::VertexInputRate::Vertex },
			{
				{
					"vertexPosition",
					0,
					Grindstone::GraphicsAPI::Format::R32G32B32_SFLOAT,
					false,
					Grindstone::GraphicsAPI::AttributeUsage::Position
				}
			}
		).Build();

	std::array<float, 3 * (8 + 80 + 16 + 17 + 4 + 32)> shapeVertices{
		// Cube
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		 // Sphere
		 0.0f, 1.0f, 0.0f,
		 0.3826834323650898f, 0.9238795325112867f, 0.0f,
		 0.7071067811865475f, 0.7071067811865476f, 0.0f,
		 0.9238795325112867f, 0.38268343236508984f, 0.0f,
		 1.0f, 0.0f, 0.0f,
		 0.9238795325112867f, -0.3826834323650897f, 0.0f,
		 0.7071067811865476f, -0.7071067811865475f, 0.0f,
		 0.3826834323650899f, -0.9238795325112867f, 0.0f,
		 0.0f, -1.0f, 0.0f,
		 -0.38268343236508967f, -0.9238795325112868f, 0.0f,
		 -0.7071067811865475f, -0.7071067811865477f, 0.0f,
		 -0.9238795325112865f, -0.38268343236509034f, 0.0f,
		 -1.0f, 0.0f, 0.0f,
		 -0.9238795325112866f, 0.38268343236509f, 0.0f,
		 -0.7071067811865477f, 0.7071067811865474f, 0.0f,
		 -0.3826834323650904f, 0.9238795325112865f, 0.0f,

		 0.0f, 0.0f, 1.0f,
		 0.0f, 0.3826834323650898f, 0.9238795325112867f,
		 0.0f, 0.7071067811865475f, 0.7071067811865476f,
		 0.0f, 0.9238795325112867f, 0.38268343236508984f,
		 0.0f, 1.0f, 0.0f,
		 0.0f, 0.9238795325112867f, -0.3826834323650897f,
		 0.0f, 0.7071067811865476f, -0.7071067811865475f,
		 0.0f, 0.3826834323650899f, -0.9238795325112867f,
		 0.0f, 0.0f, -1.0f,
		 0.0f, -0.38268343236508967f, -0.9238795325112868f,
		 0.0f, -0.7071067811865475f, -0.7071067811865477f,
		 0.0f, -0.9238795325112865f, -0.38268343236509034f,
		 0.0f, -1.0f, 0.0f,
		 0.0f, -0.9238795325112866f, 0.38268343236509f,
		 0.0f, -0.7071067811865477f, 0.7071067811865474f,
		 0.0f, -0.3826834323650904f, 0.9238795325112865f,

		 0.0f, 0.0f, 1.0f,
		 0.3826834323650898f, 0.0f, 0.9238795325112867f,
		 0.7071067811865475f, 0.0f, 0.7071067811865476f,
		 0.9238795325112867f, 0.0f, 0.38268343236508984f,
		 1.0f, 0.0f, 0.0f,
		 0.9238795325112867f, 0.0f, -0.3826834323650897f,
		 0.7071067811865476f, 0.0f, -0.7071067811865475f,
		 0.3826834323650899f, 0.0f, -0.9238795325112867f,
		 0.0f, 0.0f, -1.0f,
		 -0.38268343236508967f, 0.0f, -0.9238795325112868f,
		 -0.7071067811865475f,  0.0f, -0.7071067811865477f,
		 -0.9238795325112865f,  0.0f, -0.38268343236509034f,
		 -1.0f, 0.0f, 0.0f,
		 -0.9238795325112866f, 0.0f,0.38268343236509f,
		 -0.7071067811865477f, 0.0f,0.7071067811865474f,
		 -0.3826834323650904f, 0.0f,0.9238795325112865f,

		 0.0f, 1.0f, 0.0f,
		 0.2705980500730985f, 0.9238795325112867f, 0.2705980500730985f,
		 0.5f, 0.7071067811865476f, 0.5f,
		 0.6532814824381882f, 0.38268343236508984f, 0.6532814824381882f,
		 0.707107f, 0.0f, 0.707107f,
		 0.6532814824381882f, -0.3826834323650897f, 0.6532814824381882f,
		 0.5f, -0.7071067811865475f, 0.5f,
		 0.2705980500730985f, -0.9238795325112867f, 0.2705980500730985f,
		 0.0f, -1.0f, 0.0f,
		 -0.2705980500730985f, -0.9238795325112868f, -0.2705980500730985f,
		 -0.5f, -0.7071067811865477f, -0.5f,
		 -0.6532814824381882f, -0.38268343236509034f, -0.6532814824381882f,
		 -0.707107f, 0.0f, -0.707107f,
		 -0.6532814824381882f, 0.38268343236509f, -0.6532814824381882f,
		 -0.5f, 0.7071067811865474f, -0.5f,
		 -0.2705980500730985f, 0.9238795325112865f, -0.2705980500730985f,

		 0.0f, 1.0f, 0.0f,
		 -0.2705980500730985f, 0.9238795325112867f, 0.2705980500730985f,
		 -0.5f, 0.7071067811865476f, 0.5f,
		 -0.6532814824381882f, 0.38268343236508984f, 0.6532814824381882f,
		 -0.707107f, 0.0f, 0.707107f,
		 -0.6532814824381882f, -0.3826834323650897f, 0.6532814824381882f,
		 -0.5f, -0.7071067811865475f, 0.5f,
		 -0.2705980500730985f, -0.9238795325112867f, 0.2705980500730985f,
		 0.0f, -1.0f, 0.0f,
		 0.2705980500730985f, -0.9238795325112868f, -0.2705980500730985f,
		 0.5f, -0.7071067811865477f, -0.5f,
		 0.6532814824381882f, -0.38268343236509034f, -0.6532814824381882f,
		 0.707107f, 0.0f, -0.707107f,
		 0.6532814824381882f, 0.38268343236509f, -0.6532814824381882f,
		 0.5f, 0.7071067811865474f, -0.5f,
		 0.2705980500730985f, 0.9238795325112865f, -0.2705980500730985f,

		// Circle
		0.0f, 1.0f, 0.0f,
		0.3826834323650898f, 0.9238795325112867f, 0.0f,
		0.7071067811865475f, 0.7071067811865476f, 0.0f,
		0.9238795325112867f, 0.38268343236508984f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.9238795325112867f, -0.3826834323650897f, 0.0f,
		0.7071067811865476f, -0.7071067811865475f, 0.0f,
		0.3826834323650899f, -0.9238795325112867f, 0.0f,
		0.0f, -1.0f, 0.0f,
		-0.38268343236508967f, -0.9238795325112868f, 0.0f,
		-0.7071067811865475f, -0.7071067811865477f, 0.0f,
		-0.9238795325112865f, -0.38268343236509034f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-0.9238795325112866f, 0.38268343236509f, 0.0f,
		-0.7071067811865477f, 0.7071067811865474f, 0.0f,
		-0.3826834323650904f, 0.9238795325112865f, 0.0f,

		// Cone
		0.0f, 0.5f, 0.0f,
		0.0f, -0.5f, 1.0f,
		0.3826834323650898f, -0.5f, 0.9238795325112867f,
		0.7071067811865475f, -0.5f, 0.7071067811865476f,
		0.9238795325112867f, -0.5f, 0.38268343236508984f,
		1.0f, -0.5f, 0.0f,
		0.9238795325112867f, -0.5f, -0.3826834323650897f,
		0.7071067811865476f, -0.5f, -0.7071067811865475f,
		0.3826834323650899f, -0.5f, -0.9238795325112867f,
		0.0f, -0.5f, -1.0f,
		-0.3826834323650896f, -0.5f, -0.9238795325112868f,
		-0.7071067811865475f, -0.5f, -0.7071067811865477f,
		-0.9238795325112865f, -0.5f, -0.38268343236509034f,
		-1.0f, -0.5f, 0.0f,
		-0.9238795325112866f, -0.5f, 0.38268343236509f,
		-0.7071067811865477f, -0.5f, 0.7071067811865474f,
		-0.3826834323650904f, -0.5f, 0.9238795325112865f,

		// Plane
		-0.5, 0.0f, -0.5f,
		-0.5, 0.0f,  0.5f,
		 0.5, 0.0f,  0.5f,
		 0.5, 0.0f, -0.5f,

		 // Cylinder
		0.0f, 0.5f, 1.0f,
		0.3826834323650898f, 0.5f, 0.9238795325112867f,
		0.7071067811865475f, 0.5f, 0.7071067811865476f,
		0.9238795325112867f, 0.5f, 0.38268343236508984f,
		1.0f, 0.5f, 0.0f,
		0.9238795325112867f, 0.5f, -0.3826834323650897f,
		0.7071067811865476f, 0.5f, -0.7071067811865475f,
		0.3826834323650899f, 0.5f, -0.9238795325112867f,
		0.0f, 0.5f, -1.0f,
		-0.3826834323650896f, 0.5f, -0.9238795325112868f,
		-0.7071067811865475f, 0.5f, -0.7071067811865477f,
		-0.9238795325112865f, 0.5f, -0.38268343236509034f,
		-1.0f, 0.5f, 0.0f,
		-0.9238795325112866f, 0.5f, 0.38268343236509f,
		-0.7071067811865477f, 0.5f, 0.7071067811865474f,
		-0.3826834323650904f, 0.5f, 0.9238795325112865f,
		0.0f, -0.5f, 1.0f,
		0.3826834323650898f, -0.5f, 0.9238795325112867f,
		0.7071067811865475f, -0.5f, 0.7071067811865476f,
		0.9238795325112867f, -0.5f, 0.38268343236508984f,
		1.0f, -0.5f, 0.0f,
		0.9238795325112867f, -0.5f, -0.3826834323650897f,
		0.7071067811865476f, -0.5f, -0.7071067811865475f,
		0.3826834323650899f, -0.5f, -0.9238795325112867f,
		0.0f, -0.5f, -1.0f,
		-0.3826834323650896f, -0.5f, -0.9238795325112868f,
		-0.7071067811865475f, -0.5f, -0.7071067811865477f,
		-0.9238795325112865f, -0.5f, -0.38268343236509034f,
		-1.0f, -0.5f, 0.0f,
		-0.9238795325112866f, -0.5f, 0.38268343236509f,
		-0.7071067811865477f, -0.5f, 0.7071067811865474f,
		-0.3826834323650904f, -0.5f, 0.9238795325112865f,
	};

	std::array<uint16_t, 2 * (12 + 80 + 16 + 32 + 4 + 48)> shapeIndices{
		// Cube
		0,1,
		0,2,
		1,3,
		2,3,
		4,5,
		4,6,
		5,7,
		6,7,
		0,4,
		1,5,
		2,6,
		3,7,

		// Sphere
		0, 1,
		1, 2,
		2, 3,
		3, 4,
		4, 5,
		5, 6,
		6, 7,
		7, 8,
		8, 9,
		9, 10,
		10, 11,
		11, 12,
		12, 13,
		13, 14,
		14, 15,
		15, 0,

		16, 17,
		17, 18,
		18, 19,
		19, 20,
		20, 21,
		21, 22,
		22, 23,
		23, 24,
		24, 25,
		25, 26,
		26, 27,
		27, 28,
		28, 29,
		29, 30,
		30, 31,
		31, 16,

		32, 33,
		33, 34,
		34, 35,
		35, 36,
		36, 37,
		37, 38,
		38, 39,
		39, 40,
		40, 41,
		41, 42,
		42, 43,
		43, 44,
		44, 45,
		45, 46,
		46, 47,
		47, 32,

		48, 49,
		49, 50,
		50, 51,
		51, 52,
		52, 53,
		53, 54,
		54, 55,
		55, 56,
		56, 57,
		57, 58,
		58, 59,
		59, 60,
		60, 61,
		61, 62,
		62, 63,
		63, 48,

		64, 65,
		65, 66,
		66, 67,
		67, 68,
		68, 69,
		69, 70,
		70, 71,
		71, 72,
		72, 73,
		73, 74,
		74, 75,
		75, 76,
		76, 77,
		77, 78,
		78, 79,
		79, 64,

		// Circle
		0,1,
		1,2,
		2,3,
		3,4,
		4,5,
		5,6,
		6,7,
		7,8,
		8,9,
		9,10,
		10,11,
		11,12,
		12,13,
		13,14,
		14,15,
		15,0,

		// Cone
		1,2,
		2,3,
		3,4,
		4,5,
		5,6,
		6,7,
		7,8,
		8,9,
		9,10,
		10,11,
		11,12,
		12,13,
		13,14,
		14,15,
		15,16,
		16,1,

		1,0,
		2,0,
		3,0,
		4,0,
		5,0,
		6,0,
		7,0,
		8,0,
		9,0,
		10,0,
		11,0,
		12,0,
		13,0,
		14,0,
		15,0,
		16,0,

		// Plane
		0,1,
		1,2,
		2,3,
		3,0,

		// Cylinder
		0,1,
		1,2,
		2,3,
		3,4,
		4,5,
		5,6,
		6,7,
		7,8,
		8,9,
		9,10,
		10,11,
		11,12,
		12,13,
		13,14,
		14,15,
		15,0,

		16,17,
		17,18,
		18,19,
		19,20,
		20,21,
		21,22,
		22,23,
		23,24,
		24,25,
		25,26,
		26,27,
		27,28,
		28,29,
		29,30,
		30,31,
		31,16,

		0,16,
		1,17,
		2,18,
		3,19,
		4,20,
		5,21,
		6,22,
		7,23,
		8,24,
		9,25,
		10,26,
		11,27,
		12,28,
		13,29,
		14,30,
		15,31,
	};

	dataBuffer.resize(maxObjects);
	drawShapes.resize(maxObjects);

	GraphicsAPI::Buffer::CreateInfo ubCi{};
	ubCi.debugName = "Gizmo Uniform Buffer";
	ubCi.bufferUsage =
		GraphicsAPI::BufferUsage::TransferDst |
		GraphicsAPI::BufferUsage::TransferSrc |
		GraphicsAPI::BufferUsage::Uniform;
	ubCi.memoryUsage = GraphicsAPI::MemUsage::CPUToGPU;
	ubCi.bufferSize = static_cast<size_t>(sizeof(GizmoUniformBuffer) * maxObjects);
	gizmoUniformBuffer = graphicsCore->CreateBuffer(ubCi);

	GraphicsAPI::Buffer::CreateInfo gizmoShapesVertexBufferCi{};
	gizmoShapesVertexBufferCi.debugName = "Gizmo Shape Vertex Buffer";
	gizmoShapesVertexBufferCi.content = shapeVertices.data();
	gizmoShapesVertexBufferCi.bufferSize = sizeof(shapeVertices);
	gizmoShapesVertexBufferCi.bufferUsage =
		GraphicsAPI::BufferUsage::Vertex |
		GraphicsAPI::BufferUsage::TransferSrc |
		GraphicsAPI::BufferUsage::TransferDst;
	gizmoShapesVertexBuffer = graphicsCore->CreateBuffer(gizmoShapesVertexBufferCi);

	GraphicsAPI::Buffer::CreateInfo gizmoShapesIndexBufferCi{};
	gizmoShapesIndexBufferCi.debugName = "Gizmo Shape Index Buffer";
	gizmoShapesIndexBufferCi.content = shapeIndices.data();
	gizmoShapesIndexBufferCi.bufferSize = sizeof(shapeIndices);
	gizmoShapesIndexBufferCi.bufferUsage =
		GraphicsAPI::BufferUsage::Index |
		GraphicsAPI::BufferUsage::TransferSrc |
		GraphicsAPI::BufferUsage::TransferDst;
	gizmoShapesIndexBuffer = graphicsCore->CreateBuffer(gizmoShapesIndexBufferCi);

	GraphicsAPI::VertexArrayObject::CreateInfo vaoCi{};
	vaoCi.debugName = "Gizmo Shape Vertex Array Object";
	vaoCi.vertexBuffers = &gizmoShapesVertexBuffer;
	vaoCi.vertexBufferCount = 1;
	vaoCi.indexBuffer = gizmoShapesIndexBuffer;
	vaoCi.layout = gizmoVertexLayout;
	gizmoShapesVao = graphicsCore->CreateVertexArrayObject(vaoCi);

	uint32_t indexOffset = 0;
	uint32_t vertexOffset = 0;
	shapeMetaData[static_cast<size_t>(ShapeType::Undefined)	] = { 0, 0, 0 };
	AppendData(ShapeType::Cube, vertexOffset, indexOffset, 8, 12);
	AppendData(ShapeType::Sphere, vertexOffset, indexOffset, 80, 80);
	AppendData(ShapeType::Circle, vertexOffset, indexOffset, 16, 16);
	AppendData(ShapeType::Cone, vertexOffset, indexOffset, 17, 32);
	AppendData(ShapeType::Plane, vertexOffset, indexOffset, 4, 4);
	AppendData(ShapeType::Cyclinder, vertexOffset, indexOffset, 32, 48);

	// TODO: Capsule is currently just copying cylinder. We need to actually handle the capsule
	shapeMetaData[static_cast<size_t>(ShapeType::Capsule)] = shapeMetaData[static_cast<size_t>(ShapeType::Cyclinder)];

	std::vector<GraphicsAPI::GraphicsPipeline::ShaderStageData> shaderStageCreateInfos;
	std::vector<std::vector<char>> fileData;

	Grindstone::Assets::AssetManager* assetManager = engineCore.assetManager;
	uint8_t shaderBits = static_cast<uint8_t>(GraphicsAPI::ShaderStageBit::Vertex | GraphicsAPI::ShaderStageBit::Fragment);

	gizmoPipelineSet = assetManager->GetAssetReferenceByAddress<Grindstone::GraphicsPipelineAsset>("@CORESHADERS/editor/gizmo");

	GraphicsAPI::DescriptorSetLayout::Binding gizmoDescriptorLayoutBinding
		{ 0, 1, GraphicsAPI::BindingType::UniformBuffer, GraphicsAPI::ShaderStageBit::Vertex };

	GraphicsAPI::DescriptorSetLayout::CreateInfo gizmoDescriptorSetLayoutCreateInfo{};
	gizmoDescriptorSetLayoutCreateInfo.debugName = "Gizmo Descriptor Layout";
	gizmoDescriptorSetLayoutCreateInfo.bindingCount = 1u;
	gizmoDescriptorSetLayoutCreateInfo.bindings = &gizmoDescriptorLayoutBinding;
	gizmoDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(gizmoDescriptorSetLayoutCreateInfo);

	GraphicsAPI::DescriptorSet::Binding gizmoDescriptorBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( gizmoUniformBuffer );

	GraphicsAPI::DescriptorSet::CreateInfo gizmoDescriptorSetCreateInfo{};
	gizmoDescriptorSetCreateInfo.debugName = "Gizmo Descriptor";
	gizmoDescriptorSetCreateInfo.bindingCount = 1u;
	gizmoDescriptorSetCreateInfo.bindings = &gizmoDescriptorBinding;
	gizmoDescriptorSetCreateInfo.layout = gizmoDescriptorSetLayout;
	gizmoDescriptorSet = graphicsCore->CreateDescriptorSet(gizmoDescriptorSetCreateInfo);
}

void GizmoRenderer::SubmitCubeGizmo(const glm::mat4& transform, glm::vec3 size, glm::vec4 color) {
	drawShapes[drawCount] = ShapeType::Cube;
	dataBuffer[drawCount] = { transform * glm::scale(size) , color};
	++drawCount;
}

void GizmoRenderer::SubmitCapsuleGizmo(const glm::mat4& transform, float height, float radius, glm::vec4 color) {
	drawShapes[drawCount] = ShapeType::Capsule;
	dataBuffer[drawCount] = { transform * glm::scale(glm::vec3(radius, height, radius)) , color};
	++drawCount;
}

// TODO: Fix the transform
void GizmoRenderer::SubmitPlaneGizmo(const glm::mat4& transform, glm::vec3 normal, float positionAlongNormal, glm::vec4 color) {
	drawShapes[drawCount] = ShapeType::Plane;
	dataBuffer[drawCount] = { transform , color };
	++drawCount;
}

void GizmoRenderer::SubmitSphereGizmo(const glm::mat4& transform, float radius, glm::vec4 color) {
	drawShapes[drawCount] = ShapeType::Sphere;
	dataBuffer[drawCount] = { transform * glm::scale(glm::vec3(radius, radius, radius)) , color};
	++drawCount;
}

void GizmoRenderer::Render(Grindstone::GraphicsAPI::CommandBuffer* commandBuffer, glm::mat4 projView) {
	Grindstone::GraphicsPipelineAsset* pipelineAsset = gizmoPipelineSet.Get();
	if (pipelineAsset == nullptr) {
		return;
	}

	Grindstone::GraphicsAPI::GraphicsPipeline* gizmoPipeline = pipelineAsset->GetFirstPassPipeline(&gizmoVertexLayout);
	if (gizmoPipeline == nullptr) {
		return;
	}

	for (auto& data : dataBuffer) {
		data.transform = projView * data.transform;
	}

	gizmoUniformBuffer->UploadData(dataBuffer.data());
	commandBuffer->BindGraphicsPipeline(gizmoPipeline);
	commandBuffer->BindGraphicsDescriptorSet(gizmoPipeline, &gizmoDescriptorSet, 0, 1);
	commandBuffer->BindVertexArrayObject(gizmoShapesVao);

	for (uint32_t i = 0; i < drawCount; ++i) {
		ShapeMetaData& metaData = shapeMetaData[static_cast<size_t>(drawShapes[i])];
		commandBuffer->DrawIndices(metaData.firstIndex, metaData.indexCount, i, 1, metaData.vertexOffset);
	}

	drawCount = 0;
}
