#include <glm/gtx/transform.hpp>

#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/VertexArrayObject.hpp>
#include <Common/Graphics/CommandBuffer.hpp>
#include <Common/Graphics/UniformBuffer.hpp>
#include <Common/Graphics/DescriptorSet.hpp>
#include <Common/Graphics/DescriptorSetLayout.hpp>
#include <Common/Graphics/GraphicsPipeline.hpp>
#include <Common/Graphics/DescriptorSetLayout.hpp>
#include <Common/Graphics/VertexBuffer.hpp>
#include <Common/Graphics/IndexBuffer.hpp>
#include <Common/Graphics/IndexBuffer.hpp>
#include <Common/Graphics/Formats.hpp>
#include <Editor/EditorManager.hpp>

#include "GridRenderer.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor;

struct GridUniformBuffer {
	glm::mat4 projMatrix;
	glm::mat4 viewMatrix;
	glm::vec4 colorXAxis = glm::vec4(1.0f, 0.2f, 0.2f, 1.0f);
	glm::vec4 colorZAxis = glm::vec4(0.2f, 0.2f, 1.0f, 1.0f);
	glm::vec4 colorMinor = glm::vec4(0.2f, 0.2f, 0.2f, 0.5f);
	glm::vec4 colorMajor = glm::vec4(0.2f, 0.2f, 0.2f, 0.9f);
	glm::vec2 renderScale;
	float fadeDistanceMultiplier = 0.0f;
	float nearDistance;
	float farDistance;
};

void GridRenderer::Initialize(GraphicsAPI::RenderPass* renderPass) {
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	GraphicsAPI::UniformBuffer::CreateInfo ubCi{};
	ubCi.debugName = "Gizmo Uniform Buffer";
	ubCi.isDynamic = true;
	ubCi.size = static_cast<uint32_t>(sizeof(GridUniformBuffer));
	gridUniformBuffer = graphicsCore->CreateUniformBuffer(ubCi);

	std::vector<GraphicsAPI::ShaderStageCreateInfo> shaderStageCreateInfos;
	std::vector<std::vector<char>> fileData;

	Grindstone::Assets::AssetManager* assetManager = engineCore.assetManager;
	uint8_t shaderBits = static_cast<uint8_t>(GraphicsAPI::ShaderStageBit::Vertex | GraphicsAPI::ShaderStageBit::Fragment);

	if (!assetManager->LoadShaderSet(Uuid("49d8949b-11f9-419f-a963-60dccf89cffc"), shaderBits, 2, shaderStageCreateInfos, fileData)) {
		GPRINT_ERROR(LogSource::Rendering, "Could not load grid shaders.");
		return;
	}

	GraphicsAPI::DescriptorSetLayout::Binding gridDescriptorLayoutBinding
		{ 0, 1, GraphicsAPI::BindingType::UniformBuffer, GraphicsAPI::ShaderStageBit::Vertex | GraphicsAPI::ShaderStageBit::Fragment };

	GraphicsAPI::DescriptorSetLayout::CreateInfo gridDescriptorSetLayoutCreateInfo{};
	gridDescriptorSetLayoutCreateInfo.debugName = "Grid Descriptor Layout";
	gridDescriptorSetLayoutCreateInfo.bindingCount = 1u;
	gridDescriptorSetLayoutCreateInfo.bindings = &gridDescriptorLayoutBinding;
	gridDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(gridDescriptorSetLayoutCreateInfo);

	GraphicsAPI::DescriptorSet::Binding gridDescriptorBinding{ gridUniformBuffer };

	GraphicsAPI::DescriptorSet::CreateInfo gridDescriptorSetCreateInfo{};
	gridDescriptorSetCreateInfo.debugName = "Grid Descriptor";
	gridDescriptorSetCreateInfo.bindingCount = 1u;
	gridDescriptorSetCreateInfo.bindings = &gridDescriptorBinding;
	gridDescriptorSetCreateInfo.layout = gridDescriptorSetLayout;
	gridDescriptorSet = graphicsCore->CreateDescriptorSet(gridDescriptorSetCreateInfo);

	GraphicsAPI::GraphicsPipeline::CreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.primitiveType = GraphicsAPI::GeometryType::Triangles;
	pipelineCreateInfo.polygonFillMode = GraphicsAPI::PolygonFillMode::Fill;
	pipelineCreateInfo.cullMode = GraphicsAPI::CullMode::None;
	pipelineCreateInfo.hasDynamicScissor = true;
	pipelineCreateInfo.hasDynamicViewport = true;
	pipelineCreateInfo.vertexBindings = nullptr;
	pipelineCreateInfo.vertexBindingsCount = 0;
	pipelineCreateInfo.debugName = "Grid Pipeline";
	pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
	pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
	pipelineCreateInfo.descriptorSetLayouts = &gridDescriptorSetLayout;
	pipelineCreateInfo.descriptorSetLayoutCount = 1u;
	pipelineCreateInfo.colorAttachmentCount = 1;
	pipelineCreateInfo.isDepthWriteEnabled = false;
	pipelineCreateInfo.isDepthTestEnabled = true;
	pipelineCreateInfo.isStencilEnabled = false;
	pipelineCreateInfo.blendMode = GraphicsAPI::BlendMode::AdditiveAlpha;
	pipelineCreateInfo.renderPass = renderPass;
	gridPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
}

void GridRenderer::Render(Grindstone::GraphicsAPI::CommandBuffer* commandBuffer, glm::vec2 renderScale, glm::mat4 proj, glm::mat4 view, float nearDist, float farDist, glm::quat rotation, float offset) {
	GridUniformBuffer gridData{};
	gridData.projMatrix = proj;
	gridData.viewMatrix = view;
	gridData.renderScale = renderScale;
	gridData.nearDistance = nearDist;
	gridData.farDistance = farDist;

	gridUniformBuffer->UpdateBuffer(&gridData);
	commandBuffer->BindGraphicsPipeline(gridPipeline);
	commandBuffer->BindGraphicsDescriptorSet(gridPipeline, &gridDescriptorSet, 1);

	commandBuffer->DrawVertices(6, 0, 1, 0);
}
