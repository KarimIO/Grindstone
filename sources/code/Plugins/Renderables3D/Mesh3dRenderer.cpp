#include <algorithm>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Mesh3dRenderer.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Graphics/Core.hpp"
#include "EngineCore/Assets/Materials/MaterialImporter.hpp"
#include "EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/MeshRendererComponent.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

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

struct RenderTaskGroup {
	uint32_t offset;
	uint32_t count;
};

using RenderTaskList = std::vector<RenderTask>;
using SortedRender = std::vector<size_t>;

struct RenderSortData {
	size_t renderTaskIndex;
	uint32_t sortData;
};

struct Mesh3dUbo {
	glm::mat4 modelMatrix;
};


static int CompareRenderSort(const RenderTask& a, const RenderTask& b) {
	return a.sortData > b.sortData;
}

static int CompareReverseRenderSort(const RenderTask& a, const RenderTask& b) {
	return a.sortData < b.sortData;
}

Grindstone::Mesh3dRenderer::Mesh3dRenderer(EngineCore* engineCore) {
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
	perDrawDescriptorSetLayout = engineCore->GetGraphicsCore()->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

}

void Mesh3dRenderer::SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) {
	engineDescriptorSet = descriptorSet;
}

std::string Mesh3dRenderer::GetName() const {
	return rendererName;
}

void Mesh3dRenderer::RenderQueue(
	GraphicsAPI::CommandBuffer* commandBuffer,
	const entt::registry& registry,
	Grindstone::HashedString renderQueueHash
) {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();
	Assets::AssetManager* assetManager = engineCore->assetManager;

	std::vector<RenderTask> renderTasks;
	renderTasks.reserve(1000);

	auto view = registry.view<const entt::entity, const TransformComponent, const MeshComponent, const MeshRendererComponent>();
	view.each(
		[&](
			const entt::entity entity,
			const TransformComponent& transformComponent,
			const MeshComponent& meshComponent,
			const MeshRendererComponent& meshRenderComponent
		) {
			// Add to Render Queue
			Mesh3dAsset* meshAsset = assetManager->GetAssetByUuid<Mesh3dAsset>(meshComponent.mesh.uuid);

			if (meshAsset == nullptr) {
				return;
			}

			// Early Frustum Cull

			Math::Matrix4 transform = TransformComponent::GetWorldTransformMatrix(entity, registry);
			meshRenderComponent.perDrawUniformBuffer->UploadData(&transform);

			const std::vector<Grindstone::AssetReference<Grindstone::MaterialAsset>>& materials = meshRenderComponent.materials;
			for (const Grindstone::Mesh3dAsset::Submesh& submesh : meshAsset->submeshes) {
				if (submesh.materialIndex >= materials.size()) {
					continue;
				}

				const Grindstone::AssetReference<Grindstone::MaterialAsset>& materialReference = materials[submesh.materialIndex];
				const MaterialAsset* materialAsset = materialReference.Get();
				if (materialAsset == nullptr) {
					continue;
				}

				const GraphicsPipelineAsset* graphicsPipelineAsset = materialAsset->pipelineSetAsset.Get();
				if (graphicsPipelineAsset == nullptr) {
					continue;
				}

				const GraphicsAPI::GraphicsPipeline* pipeline = graphicsPipelineAsset->GetPassPipeline(renderQueueHash, &meshAsset->vertexArrayObject->GetLayout());
				if (pipeline == nullptr) {
					continue;
				}

				uint32_t sortData = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pipeline));

				RenderTask renderTask{};
				renderTask.materialDescriptorSet = materialAsset->materialDescriptorSet;
				renderTask.pipeline = pipeline;
				renderTask.vertexArrayObject = meshAsset->vertexArrayObject;
				renderTask.indexCount = submesh.indexCount;
				renderTask.baseVertex = submesh.baseVertex;
				renderTask.baseIndex = submesh.baseIndex;
				renderTask.perDrawDescriptorSet = meshRenderComponent.perDrawDescriptorSet;
				renderTask.sortData = sortData;

				renderTasks.emplace_back(renderTask);
			}
		}
	);

	std::sort(renderTasks.begin(), renderTasks.end(), CompareRenderSort);
	
	std::vector<RenderTaskGroup> renderTaskGroups;
	renderTasks.reserve(1000);

	const GraphicsAPI::GraphicsPipeline* graphicsPipeline = nullptr;

	for (RenderTask& renderTask : renderTasks) {
		if (graphicsPipeline != renderTask.pipeline) {
			graphicsPipeline = renderTask.pipeline;
			commandBuffer->BindGraphicsPipeline(renderTask.pipeline);
		}

		commandBuffer->BindVertexArrayObject(renderTask.vertexArrayObject);
		std::array<GraphicsAPI::DescriptorSet*, 3> descriptors = {
			engineDescriptorSet,
			renderTask.materialDescriptorSet,
			renderTask.perDrawDescriptorSet,
		};
		commandBuffer->BindGraphicsDescriptorSet(
			graphicsPipeline,
			descriptors.data(),
			0,
			static_cast<uint32_t>(descriptors.size())
		);

		commandBuffer->DrawIndices(
			renderTask.baseIndex,
			renderTask.indexCount,
			0,
			1,
			renderTask.baseVertex
		);
	}
}

GraphicsAPI::DescriptorSetLayout* Mesh3dRenderer::GetPerDrawDescriptorSetLayout() const {
	return perDrawDescriptorSetLayout;
}
