#include <Assert.hpp>

#include <Common/Graphics/Core.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshDebugRenderer.hpp>
#include <EngineCore/EngineCore.hpp>

Grindstone::Ai::NavMeshDebugRenderer* navMeshDebugRendererSingleton = nullptr;

void Grindstone::Ai::NavMeshDebugRenderer::Initialize() {
	vertexLayout = GraphicsAPI::VertexInputLayoutBuilder().AddBinding(
		{ 0, (5 * sizeof(float) + 4), GraphicsAPI::VertexInputRate::Vertex },
		{
			{
				.name = "vertexPosition",
				.locationIndex = 0,
				.format = Grindstone::GraphicsAPI::Format::R32G32B32_SFLOAT,
				.byteOffset = 0,
				.attributeUsage = Grindstone::GraphicsAPI::AttributeUsage::Position
			},
			{
				.name = "vertexTexCoord0",
				.locationIndex = 1,
				.format = Grindstone::GraphicsAPI::Format::R32G32_SFLOAT,
				.byteOffset = 3 * sizeof(float),
				.attributeUsage = Grindstone::GraphicsAPI::AttributeUsage::TexCoord0
			},
			{
				.name = "vertexColor",
				.locationIndex = 2,
				.format = Grindstone::GraphicsAPI::Format::R8G8B8A8_UNORM,
				.byteOffset = 5 * sizeof(float),
				.attributeUsage = Grindstone::GraphicsAPI::AttributeUsage::Color
			}
		}
	).Build();

	navMeshDebugRendererSingleton = this;
}

Grindstone::Ai::NavMeshDebugRenderer* Grindstone::Ai::NavMeshDebugRenderer::GetInstance() {
	return navMeshDebugRendererSingleton;
}

void Grindstone::Ai::NavMeshDebugRenderer::Clear() {
	currentCall = nullptr;
	drawCalls.clear();
	vertices.clear();
}

void Grindstone::Ai::NavMeshDebugRenderer::BuildVertexBuffers() {
	if (vertices.size() == 0) {
		return;
	}

	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	Grindstone::GraphicsAPI::Buffer* prevVertexBuffer = vertexBuffer;
	engineCore.PushDeletion([prevVertexBuffer]() {
		Grindstone::EngineCore::GetInstance().GetGraphicsCore()->DeleteBuffer(prevVertexBuffer);
	});

	Grindstone::GraphicsAPI::Buffer::CreateInfo bufferCreateInfo{
		.debugName = "Navmesh Debug Vertex Buffer",
		.content = vertices.data(),
		.bufferSize = sizeof(DebugVertex) * vertices.size(),
		.bufferUsage = Grindstone::GraphicsAPI::BufferUsage::Vertex,
		.memoryUsage = Grindstone::GraphicsAPI::MemoryUsage::GPUToCPU
	};

	vertexBuffer = graphicsCore->CreateBuffer(bufferCreateInfo);
}

bool Grindstone::Ai::NavMeshDebugRenderer::ShouldRenderThisFrame() const {
	return vertexBuffer != nullptr;
}

void Grindstone::Ai::NavMeshDebugRenderer::DrawDebug(
	Grindstone::GraphicsAPI::CommandBuffer* commandBuffer,
	Grindstone::GraphicsPipelineAsset* pipelineAsset
) {
	Grindstone::GraphicsAPI::PipelineLayout* pipelineLayout = pipelineAsset->GetFirstPassPipelineLayout();
	commandBuffer->BindVertexBuffers(&vertexBuffer, 1);

	for (auto& drawCall : drawCalls) {
		const char* callPrim = drawCall.primitiveType == duDebugDrawPrimitives::DU_DRAW_TRIS
			? "triangles"
			: "lines";
		Grindstone::GraphicsAPI::GraphicsPipeline* navMeshPipeline = pipelineAsset->GetPassPipelineByName(callPrim, &vertexLayout);
		if (navMeshPipeline != nullptr) {
			commandBuffer->BindGraphicsPipeline(navMeshPipeline);

			commandBuffer->DrawVertices(
				drawCall.vertexCount,
				0, // firstInstance
				1, // instanceCount
				drawCall.vertexOffset
			);
		}
	}
}

void Grindstone::Ai::NavMeshDebugRenderer::depthMask(bool state) {
	isDepthTestEnabled = state;
}

void Grindstone::Ai::NavMeshDebugRenderer::texture(bool state) {
	isTextured = state;
}

void Grindstone::Ai::NavMeshDebugRenderer::begin(duDebugDrawPrimitives prim, float size) {
	GS_ASSERT(currentCall == nullptr);
	currentCall = &drawCalls.emplace_back(
		DrawCall{
			.primitiveType = prim,
			.pointOrLineSize = size,
			.depthTest = isDepthTestEnabled,
			.textured = isTextured,
			.vertexOffset = static_cast<uint32_t>(vertices.size()),
			.vertexCount = 0
		}
	);
}

void Grindstone::Ai::NavMeshDebugRenderer::vertex(const float* pos, unsigned int color) {
	GS_ASSERT(currentCall != nullptr);
	vertices.push_back({ pos[0], pos[1], pos[2], 0, 0, color });
	currentCall->vertexCount++;
}

void Grindstone::Ai::NavMeshDebugRenderer::vertex(const float x, const float y, const float z, unsigned int color) {
	GS_ASSERT(currentCall != nullptr);
	vertices.push_back({ x, y, z, 0, 0, color });
	currentCall->vertexCount++;
}

void Grindstone::Ai::NavMeshDebugRenderer::vertex(const float* pos, unsigned int color, const float* uv) {
	GS_ASSERT(currentCall != nullptr);
	vertices.push_back({ pos[0], pos[1], pos[2], uv[0], uv[1], color});
	currentCall->vertexCount++;
}

void Grindstone::Ai::NavMeshDebugRenderer::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v) {
	GS_ASSERT(currentCall != nullptr);
	vertices.push_back({ x, y, z, u, v, color });
	currentCall->vertexCount++;
}

void Grindstone::Ai::NavMeshDebugRenderer::end() {
	GS_ASSERT(currentCall != nullptr);
	currentCall = nullptr;
}

unsigned int Grindstone::Ai::NavMeshDebugRenderer::areaToCol(unsigned int area) {
	return duRGBA(0, 192, 255, 255);
}
