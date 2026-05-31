#include <glm/gtx/transform.hpp>

#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Window/WindowManager.hpp>
#include <Common/Graphics/WindowGraphicsBinding.hpp>
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

const size_t boxVertexCount = 8u;
const size_t boxOutlineIndexCount = 24u;
const size_t boxFillIndexCount = 36u;

const size_t sphereSectorCount = 20u;
const size_t sphereStackCount = 20u;
static_assert(sphereSectorCount % 2 == 0);
static_assert(sphereStackCount % 4 == 0);
const size_t sphereVertexCount = (sphereSectorCount + 1u) * (sphereStackCount + 1u) * 3u;
// Full wireframe outline index count: (sphereSectorCount * (sphereStackCount - 1) * 4) + (sphereSectorCount * 2);
const size_t sphereOutlineIndexCount = ((sphereSectorCount + 1u) * 2u) + (sphereStackCount * 4u * 2u);
const size_t sphereFillIndexCount = sphereSectorCount * (sphereStackCount - 1) * 6u;

const size_t circleVertexCount = 20u;
const size_t circleOutlineIndexCount = circleVertexCount * 2u;
const size_t circleFillIndexCount = 3u * (circleVertexCount - 2u);

const size_t cylinderEdgeCount = 20u;
static_assert(cylinderEdgeCount % 4 == 0);
const size_t cylinderVertexCount = cylinderEdgeCount * 2u; // One for each cap = 2
const size_t cylinderPerCapOutlineIndexCount = cylinderEdgeCount * 2u;
const size_t cylinderPerCapFillIndexCount = 3u * (cylinderEdgeCount - 2u);
const size_t cylinderSideOutlineIndexCount = 4u * 2u; // We just want four lines around for simplicity
const size_t cylinderSideFillIndexCount = 3u * 2u * cylinderEdgeCount; // Two triangles per face, three indices per face.
const size_t cylinderOutlineIndexCount = (2 * cylinderPerCapOutlineIndexCount) + cylinderSideOutlineIndexCount;
const size_t cylinderFillIndexCount = (2 * cylinderPerCapFillIndexCount) + cylinderSideFillIndexCount;

const size_t planeVertexCount = 12u;
const size_t planeOutlineIndexCount = 8u;
const size_t planeFillIndexCount = 6u;

const size_t vertexCount = 3 * (
	boxVertexCount +
	sphereVertexCount +
	circleVertexCount +
	planeVertexCount +
	cylinderVertexCount
);

const size_t outlineIndexCount =
	boxOutlineIndexCount +
	sphereOutlineIndexCount +
	circleOutlineIndexCount +
	planeOutlineIndexCount +
	cylinderOutlineIndexCount;

const size_t fillIndexCount =
	boxFillIndexCount +
	sphereFillIndexCount +
	circleFillIndexCount +
	planeFillIndexCount +
	cylinderFillIndexCount;

static void GenerateCircle(
	size_t circleVertexCount,
	float* circleVertices,
	uint16_t* circleOutlineIndices,
	uint16_t* circleFillIndices
) {
	float theta = 2 * glm::pi<float>() / static_cast<float>(circleVertexCount);
	float angle = 0;

	size_t vertexIndex = 0;
	for (size_t i = 0; i < circleVertexCount; ++i) {
		circleVertices[vertexIndex++] = std::cos(angle);
		circleVertices[vertexIndex++] = std::sin(angle);
		circleVertices[vertexIndex++] = 0.0f;
		angle += theta;
	}

	size_t outlineIndex = 0;
	size_t fillIndex = 0;
	// Face count = verts - 2, Outline count = verts, so generate the last two faces after.
	for (uint16_t i = 0; i < circleVertexCount - 2u; ++i) {
		circleOutlineIndices[outlineIndex++] = i;
		circleOutlineIndices[outlineIndex++] = i + 1u;
		circleFillIndices[fillIndex++] = 0;
		circleFillIndices[fillIndex++] = i + 1u;
		circleFillIndices[fillIndex++] = i + 2u;
	}

	circleOutlineIndices[outlineIndex++] = circleVertexCount - 1u;
	circleOutlineIndices[outlineIndex++] = circleVertexCount;
	circleOutlineIndices[outlineIndex++] = circleVertexCount;
	circleOutlineIndices[outlineIndex++] = 0u;

	GS_ASSERT_ENGINE(fillIndex == circleFillIndexCount);
	GS_ASSERT_ENGINE(outlineIndex == circleOutlineIndexCount);
}

static void GenerateCylinder(
	size_t cylinderEdgeCount,
	float* cylinderVertices,
	uint16_t* cylinderOutlineIndices,
	uint16_t* cylinderFillIndices
) {
	size_t vertsPerCap = cylinderEdgeCount * 3u;
	float theta = 2 * glm::pi<float>() / static_cast<float>(cylinderEdgeCount);
	float angle = 0;

	size_t vertexIndex = 0;
	for (size_t i = 0; i < cylinderEdgeCount; ++i) {
		size_t top = vertexIndex;
		size_t bottom = vertexIndex + vertsPerCap;

		float x = std::cos(angle);
		float y = std::sin(angle);

		cylinderVertices[top + 0] = x;
		cylinderVertices[top + 1] = 0.5;
		cylinderVertices[top + 2] = y;

		cylinderVertices[bottom + 0] = x;
		cylinderVertices[bottom + 1] = -0.5;
		cylinderVertices[bottom + 2] = y;

		vertexIndex += 3;

		angle += theta;
	}

	size_t outlineIndex = 0;
	size_t fillIndex = 0;

	// Generate Caps
	for (uint16_t c = 0; c < 2; ++c) {
		uint16_t offset = c * cylinderEdgeCount;
		// Face count = verts - 2, Outline count = verts, so generate the last two faces after.
		for (uint16_t i = 0; i < cylinderEdgeCount - 2u; ++i) {
			cylinderOutlineIndices[outlineIndex++] = offset + i;
			cylinderOutlineIndices[outlineIndex++] = offset + i + 1u;

			// Swapped to reverse winding order for faces.
			if (c == 0) {
				cylinderFillIndices[fillIndex++] = offset;
				cylinderFillIndices[fillIndex++] = offset + i + 2u;
				cylinderFillIndices[fillIndex++] = offset + i + 1u;
			}
			else {
				cylinderFillIndices[fillIndex++] = offset;
				cylinderFillIndices[fillIndex++] = offset + i + 1u;
				cylinderFillIndices[fillIndex++] = offset + i + 2u;
			}
		}

		cylinderOutlineIndices[outlineIndex++] = offset + cylinderEdgeCount - 2u;
		cylinderOutlineIndices[outlineIndex++] = offset + cylinderEdgeCount - 1u;
		cylinderOutlineIndices[outlineIndex++] = offset + cylinderEdgeCount - 1u;
		cylinderOutlineIndices[outlineIndex++] = offset;
	}

	// Generate Fill Sides
	for (uint16_t i = 0; i < cylinderEdgeCount - 1u; ++i) {
		cylinderFillIndices[fillIndex++] = i;
		cylinderFillIndices[fillIndex++] = i + 1u;
		cylinderFillIndices[fillIndex++] = i + cylinderEdgeCount;

		cylinderFillIndices[fillIndex++] = i + cylinderEdgeCount;
		cylinderFillIndices[fillIndex++] = i + 1u;
		cylinderFillIndices[fillIndex++] = i + cylinderEdgeCount + 1u;
	}

	uint16_t last = cylinderEdgeCount - 1u;
	cylinderFillIndices[fillIndex++] = last;
	cylinderFillIndices[fillIndex++] = 0;
	cylinderFillIndices[fillIndex++] = last + cylinderEdgeCount;

	cylinderFillIndices[fillIndex++] = 0;
	cylinderFillIndices[fillIndex++] = cylinderEdgeCount;
	cylinderFillIndices[fillIndex++] = last + cylinderEdgeCount;

	// Generate Outline Sides
	for (size_t i = 0; i < 4; ++i) {
		size_t offset = (cylinderEdgeCount / 4) * i;

		// Top
		cylinderOutlineIndices[outlineIndex++] = offset;
		// Bottom
		cylinderOutlineIndices[outlineIndex++] = offset + cylinderEdgeCount;
	}

	GS_ASSERT_ENGINE(fillIndex == cylinderFillIndexCount);
	GS_ASSERT_ENGINE(outlineIndex == cylinderOutlineIndexCount);
}

// Source: https://songho.ca/opengl/gl_sphere.html
static void GenerateSphere(
	size_t sectorCount,
	size_t stackCount,
	float* vertices,
	uint16_t* outlineIndices,
	uint16_t* fillIndices
) {
	float radius = 1.0f;
	float sectorStep = 2 * glm::pi<float>() / sectorCount;
	float stackStep = glm::pi<float>() / stackCount;
	float sectorAngle, stackAngle;

	size_t vertexIndex = 0;
	size_t fillIndicesIndex = 0;
	size_t outlineIndicesIndex = 0;

	for (size_t i = 0; i <= stackCount; ++i) {
		stackAngle = glm::pi<float>() / 2 - i * stackStep;  // starting from pi/2 to -pi/2
		float xz = radius * std::cos(stackAngle);			// r * cos(u)
		float y = radius * std::sin(stackAngle);				// r * sin(u)

		// add (sectorCount+1) vertices per stack
		// first and last vertices have same position and normal, but different tex coords
		for (size_t j = 0; j <= sectorCount; ++j) {
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position (x, y, z)
			float x = xz * std::cos(sectorAngle);       // r * cos(u) * cos(v)
			float z = xz * std::sin(sectorAngle);       // r * cos(u) * sin(v)
			vertices[vertexIndex++] = x;
			vertices[vertexIndex++] = y;
			vertices[vertexIndex++] = z;
		}
	}

	int k1, k2;
	for (int i = 0; i < stackCount; ++i) {
		k1 = i * (sectorCount + 1);     // beginning of current stack
		k2 = k1 + sectorCount + 1;      // beginning of next stack

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if (i != 0) {
				fillIndices[fillIndicesIndex++] = k1;
				fillIndices[fillIndicesIndex++] = k2;
				fillIndices[fillIndicesIndex++] = k1 + 1;
			}

			// k1+1 => k2 => k2+1
			if (i != (stackCount - 1)) {
				fillIndices[fillIndicesIndex++] = k1 + 1;
				fillIndices[fillIndicesIndex++] = k2;
				fillIndices[fillIndicesIndex++] = k2 + 1;
			}

			/*
				// This outline code generates outlines for every face.
				// It's commented in case we ever want to go back to this instead of the
				// three rings outline method.
				outlineIndices[outlineIndicesIndex++] = k1;
				outlineIndices[outlineIndicesIndex++] = k2;
				if (i != 0) {
					outlineIndices[outlineIndicesIndex++] = k1;
					outlineIndices[outlineIndicesIndex++] = k1 + 1;
				}
			*/

			// Render outline slices along x/z axes.
			if (j % (sectorCount / 4) == 0) {
				outlineIndices[outlineIndicesIndex++] = k1;
				outlineIndices[outlineIndicesIndex++] = k2;
			}
		}
	}

	int k = (stackCount / 2) * (sectorCount + 1);
	for (int i = 0; i < sectorCount; ++i) {
		outlineIndices[outlineIndicesIndex++] = k + i;
		outlineIndices[outlineIndicesIndex++] = k + i + 1;
	}
	outlineIndices[outlineIndicesIndex++] = k + sectorCount - 1u;
	outlineIndices[outlineIndicesIndex++] = k;

	GS_ASSERT_ENGINE(fillIndicesIndex == sphereFillIndexCount);
	GS_ASSERT_ENGINE(outlineIndicesIndex == sphereOutlineIndexCount);
}

static auto GenerateGeometryArrays() {
	std::array<float, vertexCount> shapeVertices{
		// Cube
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		 // Plane
		 -0.5f, 0.0f, -0.5f,
		 -0.5f, 0.0f,  0.5f,
		  0.5f, 0.0f,  0.5f,
		  0.5f, 0.0f, -0.5f
	};

	std::array<uint16_t, outlineIndexCount + fillIndexCount> shapeIndices{};

	std::array<uint16_t, boxOutlineIndexCount> boxOutlineIndices = {
		0,1,    0,2,
		1,3,    2,3,
		4,5,    4,6,
		5,7,    6,7,
		0,4,    1,5,
		2,6,    3,7,
	};

	std::array<uint16_t, boxFillIndexCount> boxFillIndices = {
		0, 2, 3,    0, 3, 1,
		4, 5, 7,    4, 7, 6,
		0, 4, 6,    0, 6, 2,
		1, 3, 7,    1, 7, 5,
		0, 1, 5,    0, 5, 4,
		2, 6, 7,    2, 7, 3,
	};

	std::array<uint16_t, boxOutlineIndexCount> planeOutlineIndices{ 0,1,    1,2,    2,3,    3,0 };
	std::array<uint16_t, boxFillIndexCount> planeFillIndices{ 0,1,2,    0,2,3 };

	auto circleVertexIt = shapeVertices.data() + (3u * boxVertexCount) + (3u * planeVertexCount);
	auto sphereVertexIt = circleVertexIt + (3u * circleVertexCount);
	auto cylinderVertexIt = sphereVertexIt + (3u * sphereVertexCount);

	auto idxTarget = shapeIndices.begin();
	std::copy(boxFillIndices.begin(), boxFillIndices.end(), idxTarget);
	idxTarget += boxFillIndexCount;
	std::copy(planeFillIndices.begin(), planeFillIndices.end(), idxTarget);
	idxTarget += planeFillIndexCount;
	auto circleFillIt = idxTarget;
	idxTarget += circleFillIndexCount;
	auto sphereFillIt = idxTarget;
	idxTarget += sphereFillIndexCount;
	auto cylinderFillIt = idxTarget;
	idxTarget += cylinderFillIndexCount;

	std::copy(boxOutlineIndices.begin(), boxOutlineIndices.end(), idxTarget);
	idxTarget += boxOutlineIndexCount;
	std::copy(planeOutlineIndices.begin(), planeOutlineIndices.end(), idxTarget);
	idxTarget += planeOutlineIndexCount;
	auto circleOutlineIt = idxTarget;
	idxTarget += circleOutlineIndexCount;
	auto sphereOutlineIt = idxTarget;
	idxTarget += sphereOutlineIndexCount;
	auto cylinderOutlineIt = idxTarget;
	idxTarget += cylinderOutlineIndexCount;
	assert(idxTarget == shapeIndices.end());

	GenerateCircle(
		circleVertexCount,
		circleVertexIt,
		std::to_address(circleOutlineIt),
		std::to_address(circleFillIt)
	);

	GenerateCylinder(
		cylinderEdgeCount,
		cylinderVertexIt,
		std::to_address(cylinderOutlineIt),
		std::to_address(cylinderFillIt)
	);

	GenerateSphere(
		sphereSectorCount,
		sphereStackCount,
		sphereVertexIt,
		std::to_address(sphereOutlineIt),
		std::to_address(sphereFillIt)
	);

	return std::pair(shapeVertices, shapeIndices);
}

inline void GizmoRenderer::AppendData(
	ShapeType shapeType,
	uint32_t& currentVertexOffset, uint32_t& currentOutlineIndexOffset, uint32_t& currentFilledIndexOffset,
	uint32_t vertexCount, uint32_t outlineIndexCount, uint32_t filledIndexCount
) {
	shapeMetaData[static_cast<size_t>(shapeType)] = ShapeMetaData{
		.firstOutlineIndex = currentOutlineIndexOffset,
		.outlineIndexCount = outlineIndexCount,
		.firstFilledIndex = currentFilledIndexOffset,
		.filledIndexCount = filledIndexCount,
		.vertexOffset = currentVertexOffset
	};

	currentVertexOffset += vertexCount;
	currentOutlineIndexOffset += outlineIndexCount;
	currentFilledIndexOffset += filledIndexCount;
}

void GizmoRenderer::Initialize() {
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	auto wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	uint32_t maxFramesInFlight = wgb->GetMaxFramesInFlight();

	GraphicsAPI::VertexInputLayoutBuilder layoutBuilder;
	gizmoVertexLayout = layoutBuilder
		.AddBinding(
			{ 0, 3 * sizeof(float), GraphicsAPI::VertexInputRate::Vertex},
			{
				{
					"vertexPosition",
					0,
					Grindstone::GraphicsAPI::Format::R32G32B32_SFLOAT,
					0,
					Grindstone::GraphicsAPI::AttributeUsage::Position
				}
			}
		).Build();

	auto [shapeVertices, shapeIndices] = GenerateGeometryArrays();

	dataBuffer.resize(maxObjects);
	drawShapes.resize(maxObjects);

	GraphicsAPI::Buffer::CreateInfo ubCi{};
	ubCi.bufferUsage =
		GraphicsAPI::BufferUsage::TransferDst |
		GraphicsAPI::BufferUsage::TransferSrc |
		GraphicsAPI::BufferUsage::Uniform;
	ubCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
	ubCi.bufferSize = static_cast<size_t>(sizeof(GizmoUniformBuffer) * maxObjects);

	for (uint32_t i = 0; i < maxFramesInFlight; ++i) {
		std::string debugName = std::vformat("Gizmo Uniform Buffer [{}]", std::make_format_args(i));
		ubCi.debugName = debugName.c_str();
		gizmoUniformBuffers[i] = graphicsCore->CreateBuffer(ubCi);
	}

	GraphicsAPI::Buffer::CreateInfo gizmoShapesVertexBufferCi{
		.debugName = "Gizmo Shape Vertex Buffer",
		.content = shapeVertices.data(),
		.bufferSize = sizeof(shapeVertices),
		.bufferUsage =
			GraphicsAPI::BufferUsage::Vertex |
			GraphicsAPI::BufferUsage::TransferSrc |
			GraphicsAPI::BufferUsage::TransferDst
	};
	gizmoShapesVertexBuffer = graphicsCore->CreateBuffer(gizmoShapesVertexBufferCi);

	GraphicsAPI::Buffer::CreateInfo gizmoShapesIndexBufferCi{
		.debugName = "Gizmo Shape Index Buffer",
		.content = shapeIndices.data(),
		.bufferSize = sizeof(shapeIndices),
		.bufferUsage =
			GraphicsAPI::BufferUsage::Index |
			GraphicsAPI::BufferUsage::TransferSrc |
			GraphicsAPI::BufferUsage::TransferDst
	};
	gizmoShapesIndexBuffer = graphicsCore->CreateBuffer(gizmoShapesIndexBufferCi);

	GraphicsAPI::VertexArrayObject::CreateInfo vaoCi{};
	vaoCi.debugName = "Gizmo Shape Vertex Array Object";
	vaoCi.vertexBuffers = &gizmoShapesVertexBuffer;
	vaoCi.vertexBufferCount = 1;
	vaoCi.indexBuffer = gizmoShapesIndexBuffer;
	vaoCi.layout = gizmoVertexLayout;
	gizmoShapesVao = graphicsCore->CreateVertexArrayObject(vaoCi);

	uint32_t filledIndexOffset = 0;
	uint32_t outlineIndexOffset = fillIndexCount;
	uint32_t vertexOffset = 0;
	shapeMetaData[static_cast<size_t>(ShapeType::Undefined)	] = { 0, 0, 0 };
	AppendData(ShapeType::Cube, vertexOffset, outlineIndexOffset, filledIndexOffset, boxVertexCount, boxOutlineIndexCount, boxFillIndexCount);
	AppendData(ShapeType::Plane, vertexOffset, outlineIndexOffset, filledIndexOffset, planeVertexCount, planeOutlineIndexCount, planeFillIndexCount);
	AppendData(ShapeType::Circle, vertexOffset, outlineIndexOffset, filledIndexOffset, circleVertexCount, circleOutlineIndexCount, circleFillIndexCount);
	AppendData(ShapeType::Sphere, vertexOffset, outlineIndexOffset, filledIndexOffset, sphereVertexCount, sphereOutlineIndexCount, sphereFillIndexCount);
	AppendData(ShapeType::Cyclinder, vertexOffset, outlineIndexOffset, filledIndexOffset, cylinderVertexCount, cylinderOutlineIndexCount, cylinderFillIndexCount);
	
	// TODO: Capsule is currently just copying cylinder. We need to actually handle the capsule
	shapeMetaData[static_cast<size_t>(ShapeType::Capsule)] = shapeMetaData[static_cast<size_t>(ShapeType::Cyclinder)];
	shapeMetaData[static_cast<size_t>(ShapeType::Cone)] = shapeMetaData[static_cast<size_t>(ShapeType::Cyclinder)];

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
	gizmoDescriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(gizmoDescriptorSetLayoutCreateInfo);


	GraphicsAPI::DescriptorSet::CreateInfo gizmoDescriptorSetCreateInfo{};
	gizmoDescriptorSetCreateInfo.bindingCount = 1u;
	gizmoDescriptorSetCreateInfo.layout = gizmoDescriptorSetLayout;

	for (uint32_t i = 0; i < maxFramesInFlight; ++i) {
		std::string debugName = std::vformat("Gizmo Descriptor [{}]", std::make_format_args(i));
		gizmoDescriptorSetCreateInfo.debugName = debugName.c_str();
		GraphicsAPI::DescriptorSet::Binding gizmoDescriptorBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( gizmoUniformBuffers[i]);
		gizmoDescriptorSetCreateInfo.bindings = &gizmoDescriptorBinding;
		gizmoDescriptorSets[i] = graphicsCore->CreateDescriptorSet(gizmoDescriptorSetCreateInfo);
	}
}

static glm::vec4 MakeFillColor(const glm::vec4& srcColor) {
	return glm::vec4(srcColor.r, srcColor.g, srcColor.b, srcColor.a * 0.3f);
}

void GizmoRenderer::SubmitCubeGizmo(const glm::mat4& transform, glm::vec3 size, glm::vec4 color) {
	drawShapes[drawCount] = ShapeType::Cube;
	dataBuffer[drawCount] = { transform * glm::scale(size), color, MakeFillColor(color) };
	++drawCount;
}

void GizmoRenderer::SubmitCapsuleGizmo(const glm::mat4& transform, float height, float radius, glm::vec4 color) {
	drawShapes[drawCount] = ShapeType::Capsule;
	dataBuffer[drawCount] = { transform * glm::scale(glm::vec3(radius, height, radius)) , color, MakeFillColor(color) };
	++drawCount;
}

// TODO: Fix the transform
void GizmoRenderer::SubmitPlaneGizmo(const glm::mat4& transform, glm::vec3 normal, float positionAlongNormal, glm::vec4 color) {
	drawShapes[drawCount] = ShapeType::Plane;
	dataBuffer[drawCount] = { transform , color, MakeFillColor(color) };
	++drawCount;
}

void GizmoRenderer::SubmitSphereGizmo(const glm::mat4& transform, float radius, glm::vec4 color) {
	drawShapes[drawCount] = ShapeType::Sphere;
	dataBuffer[drawCount] = { transform * glm::scale(glm::vec3(radius, radius, radius)) , color, MakeFillColor(color) };
	++drawCount;
}

void GizmoRenderer::Render(Grindstone::GraphicsAPI::CommandBuffer* commandBuffer, glm::mat4 projView) {
	size_t currentDrawCount = drawCount;
	drawCount = 0;

	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	auto wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	uint32_t imageIndex = wgb->GetCurrentImageIndex();

	Grindstone::GraphicsPipelineAsset* pipelineAsset = gizmoPipelineSet.Get();
	if (pipelineAsset == nullptr) {
		return;
	}

	for (auto& data : dataBuffer) {
		data.transform = projView * data.transform;
	}

	gizmoUniformBuffers[imageIndex]->UploadData(dataBuffer.data());

	Grindstone::GraphicsAPI::PipelineLayout* pipelineLayout = pipelineAsset->GetFirstPassPipelineLayout();
	commandBuffer->BindGraphicsDescriptorSet(pipelineLayout, &gizmoDescriptorSets[imageIndex], 2, 1);
	commandBuffer->BindVertexArrayObject(gizmoShapesVao);

	// Draw Outlines
	{
		Grindstone::GraphicsAPI::GraphicsPipeline* gizmoPipeline = pipelineAsset->GetPassPipelineByName("wireframe", &gizmoVertexLayout);
		if (gizmoPipeline != nullptr) {
			commandBuffer->BindGraphicsPipeline(gizmoPipeline);

			for (uint32_t i = 0; i < currentDrawCount; ++i) {
				ShapeMetaData& metaData = shapeMetaData[static_cast<size_t>(drawShapes[i])];
				commandBuffer->DrawIndices(
					metaData.firstOutlineIndex,
					metaData.outlineIndexCount,
					i, 1,
					metaData.vertexOffset
				);
			}
		}
	}

	// Draw Fills
	{
		const Grindstone::GraphicsAPI::GraphicsPipeline* gizmoPipeline = pipelineAsset->GetPassPipelineByName("fill", &gizmoVertexLayout);
		if (gizmoPipeline != nullptr) {
			commandBuffer->BindGraphicsPipeline(gizmoPipeline);

			for (uint32_t i = 0; i < currentDrawCount; ++i) {
				ShapeMetaData& metaData = shapeMetaData[static_cast<size_t>(drawShapes[i])];
				commandBuffer->DrawIndices(
					metaData.firstFilledIndex,
					metaData.filledIndexCount,
					i, 1,
					metaData.vertexOffset
				);
			}
		}
	}
}
