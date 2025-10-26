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

#include "GridRenderer.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor;

struct GridUniformBuffer {
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 inverseProjectionMatrix;
	glm::mat4 inverseViewMatrix;
	glm::vec4 colorXAxis = glm::vec4(1.0f, 0.2f, 0.2f, 1.0f);
	glm::vec4 colorZAxis = glm::vec4(0.2f, 0.2f, 1.0f, 1.0f);
	glm::vec4 colorMinor = glm::vec4(0.2f, 0.2f, 0.2f, 0.5f);
	glm::vec4 colorMajor = glm::vec4(0.2f, 0.2f, 0.2f, 0.9f);
	glm::vec2 renderScale;
	float fadeDistanceMultiplier = 0.0f;
	float nearDistance;
	float farDistance;
};

void GridRenderer::Initialize() {
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	auto wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	uint32_t maxFramesInFlight = wgb->GetMaxFramesInFlight();

	GraphicsAPI::Buffer::CreateInfo ubCi{};
	ubCi.bufferUsage =
		GraphicsAPI::BufferUsage::TransferDst |
		GraphicsAPI::BufferUsage::TransferSrc |
		GraphicsAPI::BufferUsage::Uniform;
	ubCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
	ubCi.bufferSize = static_cast<size_t>(sizeof(GridUniformBuffer));

	for (uint32_t i = 0; i < maxFramesInFlight; ++i) {
		std::string debugName = std::vformat("Grid Uniform Buffer [{}]", std::make_format_args(i));
		ubCi.debugName = debugName.c_str();
		gridUniformBuffers[i] = graphicsCore->CreateBuffer(ubCi);
	}

	std::vector<GraphicsAPI::GraphicsPipeline::ShaderStageData> shaderStageCreateInfos;
	std::vector<std::vector<char>> fileData;

	Grindstone::Assets::AssetManager* assetManager = engineCore.assetManager;
	uint8_t shaderBits = static_cast<uint8_t>(GraphicsAPI::ShaderStageBit::Vertex | GraphicsAPI::ShaderStageBit::Fragment);

	pipelineSet = assetManager->GetAssetReferenceByAddress<Grindstone::GraphicsPipelineAsset>("@CORESHADERS/editor/grid");

	GraphicsAPI::DescriptorSetLayout::Binding gridDescriptorLayoutBinding
		{ 0, 1, GraphicsAPI::BindingType::UniformBuffer, GraphicsAPI::ShaderStageBit::Vertex | GraphicsAPI::ShaderStageBit::Fragment };

	GraphicsAPI::DescriptorSetLayout::CreateInfo gridDescriptorSetLayoutCreateInfo{};
	gridDescriptorSetLayoutCreateInfo.debugName = "Grid Descriptor Layout";
	gridDescriptorSetLayoutCreateInfo.bindingCount = 1u;
	gridDescriptorSetLayoutCreateInfo.bindings = &gridDescriptorLayoutBinding;
	gridDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(gridDescriptorSetLayoutCreateInfo);

	GraphicsAPI::DescriptorSet::CreateInfo gridDescriptorSetCreateInfo{};
	gridDescriptorSetCreateInfo.bindingCount = 1u;
	gridDescriptorSetCreateInfo.layout = gridDescriptorSetLayout;

	for (uint32_t i = 0; i < maxFramesInFlight; ++i) {
		std::string debugName = std::vformat("Grid Descriptor [{}]", std::make_format_args(i));
		gridDescriptorSetCreateInfo.debugName = debugName.c_str();
		GraphicsAPI::DescriptorSet::Binding gridDescriptorBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(gridUniformBuffers[i]);
		gridDescriptorSetCreateInfo.bindings = &gridDescriptorBinding;
		gridDescriptorSets[i] = graphicsCore->CreateDescriptorSet(gridDescriptorSetCreateInfo);
	}

}

void GridRenderer::Render(Grindstone::GraphicsAPI::CommandBuffer* commandBuffer, glm::vec2 renderScale, glm::mat4 proj, glm::mat4 view, float nearDist, float farDist, glm::quat rotation, float offset) {
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	auto wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	uint32_t imageIndex = wgb->GetCurrentImageIndex();

	Grindstone::GraphicsPipelineAsset* pipelineAsset = pipelineSet.Get();
	if (pipelineAsset == nullptr) {
		return;
	}

	Grindstone::GraphicsAPI::GraphicsPipeline* pipeline = pipelineAsset->GetFirstPassPipeline(nullptr);
	if (pipeline == nullptr) {
		return;
	}

	GridUniformBuffer gridData{};
	gridData.projectionMatrix = proj;
	gridData.viewMatrix = view;
	gridData.inverseProjectionMatrix = glm::inverse(proj);
	gridData.inverseViewMatrix = glm::inverse(view);
	gridData.renderScale = renderScale;
	gridData.nearDistance = nearDist;
	gridData.farDistance = farDist;

	gridUniformBuffers[imageIndex]->UploadData(&gridData);
	commandBuffer->BindGraphicsPipeline(pipeline);
	commandBuffer->BindGraphicsDescriptorSet(pipeline, &gridDescriptorSets[imageIndex], 2, 1);

	commandBuffer->DrawVertices(6, 0, 1, 0);
}
