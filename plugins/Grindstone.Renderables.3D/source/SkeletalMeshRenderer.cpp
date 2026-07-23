#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Common/Graphics/Core.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Assets/Materials/MaterialImporter.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <EngineCore/Scenes/Scene.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>

#include <Grindstone.Renderables.3D/include/RenderTasks.hpp>
#include <Grindstone.Renderables.3D/include/SortRenderTasks.hpp>
#include <Grindstone.Renderables.3D/include/FrustumCulling.hpp>
#include <Grindstone.Renderables.3D/include/SkeletalMeshRenderer.hpp>
#include <Grindstone.Renderables.3D/include/Components/SkeletalMeshComponent.hpp>

using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

GraphicsAPI::DescriptorSetLayout* Grindstone::SkeletalMeshRenderer::perDrawDescriptorSetLayout = nullptr;

struct RenderTask {
	GraphicsAPI::DescriptorSet* materialDescriptorSet;
	GraphicsAPI::DescriptorSet* perDrawDescriptorSet;
	const class GraphicsAPI::GraphicsPipeline* pipeline;
	const class GraphicsAPI::VertexArrayObject* vertexArrayObject;
	uint32_t indexCount;
	uint32_t baseVertex;
	uint32_t baseIndex;
	glm::mat4 transformMatrix;
	uint32_t sortData;
};

static void AppendSkeletalSubmeshRenderTask(
	std::vector<RenderTask>& renderTasks,
	const Grindstone::HashedString renderQueueHash,
	const Mesh3dAsset::Submesh& submesh,
	const Mesh3dAsset* meshAsset,
	const SkeletalMeshComponent& meshComponent,
	const MeshRendererComponent& meshRenderComponent
) {
	if (submesh.materialIndex >= meshRenderComponent.materials.size()) {
		return;
	}

	const Grindstone::AssetReference<Grindstone::MaterialAsset>& materialReference = meshRenderComponent.materials[submesh.materialIndex];
	const MaterialAsset* materialAsset = materialReference.Get();
	if (materialAsset == nullptr) {
		return;
	}

	const GraphicsPipelineAsset* graphicsPipelineAsset = materialAsset->pipelineSetAsset.Get();
	if (graphicsPipelineAsset == nullptr) {
		return;
	}

	const GraphicsAPI::GraphicsPipeline* pipeline = graphicsPipelineAsset->GetPassPipelineByRenderQueue(renderQueueHash, &meshAsset->vertexArrayObject->GetLayout());
	if (pipeline == nullptr) {
		return;
	}

	uint32_t sortData = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pipeline));

	RenderTask renderTask{
		.materialDescriptorSet = materialAsset->materialDescriptorSet,
		.perDrawDescriptorSet = meshRenderComponent.perDrawDescriptorSet,
		.pipeline = pipeline,
		.vertexArrayObject = meshComponent.skinnedVertexArrayObject,
		.indexCount = submesh.indexCount,
		.baseVertex = submesh.baseVertex,
		.baseIndex = submesh.baseIndex,
		.sortData = sortData
	};

	renderTasks.emplace_back(renderTask);
}

Grindstone::SkeletalMeshRenderer::SkeletalMeshRenderer(EngineCore* engineCore) {
	this->engineCore = engineCore;

	GraphicsAPI::DescriptorSetLayout::Binding descriptorSetUniformBinding{};
	descriptorSetUniformBinding.bindingId = 0;
	descriptorSetUniformBinding.type = GraphicsAPI::BindingType::UniformBuffer;
	descriptorSetUniformBinding.count = 1;
	descriptorSetUniformBinding.stages = GraphicsAPI::ShaderStageBit::Vertex;

	GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.debugName = "Per Draw Descriptor Set Layout";
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.bindings = &descriptorSetUniformBinding;
	perDrawDescriptorSetLayout = engineCore->GetGraphicsCore()->GetOrCreateDescriptorSetLayoutFromCache(descriptorSetLayoutCreateInfo);

}

void SkeletalMeshRenderer::SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) {
	engineDescriptorSet = descriptorSet;
}

std::string SkeletalMeshRenderer::GetName() const {
	return rendererName;
}

Grindstone::Rendering::GeometryRenderStats SkeletalMeshRenderer::RenderQueue(
	GraphicsAPI::CommandBuffer* commandBuffer,
	const Grindstone::Rendering::RenderViewData& renderViewData,
	entt::registry& registry,
	Grindstone::HashedString renderQueueHash
) {
	Grindstone::Rendering::GeometryRenderStats renderingStats{};
	Grindstone::Renderer::CullingFrustum frustum = Grindstone::Renderer::CreateFrustum(renderViewData);

	glm::mat4 viewMatrix = renderViewData.viewMatrix;

	std::chrono::time_point start = std::chrono::steady_clock::now();

	std::vector<RenderTask> renderTasks = Grindstone::Renderer::GenerateTaskList<SkeletalMeshComponent, RenderTask>(
		renderingStats,
		registry,
		frustum,
		viewMatrix,
		renderQueueHash,
		AppendSkeletalSubmeshRenderTask
	);
	Grindstone::Renderer::SortRenderTasks<RenderTask>(renderTasks);
	Grindstone::Renderer::RenderAllTasks<RenderTask>(renderingStats, engineDescriptorSet, commandBuffer, renderTasks);

	std::chrono::time_point end = std::chrono::steady_clock::now();
	long long ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	renderingStats.cpuTimeMs = static_cast<double>(ns) * 0.000001;

	return renderingStats;
}

GraphicsAPI::DescriptorSetLayout* SkeletalMeshRenderer::GetPerDrawDescriptorSetLayout() {
	return perDrawDescriptorSetLayout;
}
