#include <algorithm>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Mesh3dRenderer.hpp"
#include "EngineCore/EngineCore.hpp"
#include "Common/Graphics/Core.hpp"
#include "EngineCore/Assets/Materials/MaterialImporter.hpp"
#include "EngineCore/Assets/Shaders/ShaderAsset.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/MeshRendererComponent.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

struct Mesh3dUbo {
	glm::mat4 modelMatrix;
};

static int CompareRenderSort(const RenderSortData& a, const RenderSortData& b) {
	return a.sortData > b.sortData;
}

static int CompareReverseRenderSort(const RenderSortData& a, const RenderSortData& b) {
	return a.sortData < b.sortData;
}

void Mesh3dRenderer::AddQueue(const char* queueName, DrawSortMode drawSortMode) {
	RenderQueueIndex index = static_cast<RenderQueueIndex>(renderQueues.size());
	RenderQueueContainer& queue = renderQueues.emplace_back(drawSortMode);
	renderQueueMap[queueName] = index;
}

Grindstone::Mesh3dRenderer::Mesh3dRenderer(EngineCore* engineCore) {
	this->engineCore = engineCore;


	GraphicsAPI::DescriptorSetLayout::Binding descriptorSetUniformBinding{};
	descriptorSetUniformBinding.bindingId = 0;
	descriptorSetUniformBinding.type = GraphicsAPI::BindingType::UniformBuffer;
	descriptorSetUniformBinding.count = 1;
	descriptorSetUniformBinding.stages = GraphicsAPI::ShaderStageBit::Vertex | GraphicsAPI::ShaderStageBit::Fragment;

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

void Mesh3dRenderer::ValidateMeshRenderer(MeshRendererComponent& meshRenderComponent) {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();

	// TODO: Where can I put this? It needs to be done every time a meshrenderer is added.
	// Maybe just make an array of perDrawUniformBuffers
	if (meshRenderComponent.perDrawUniformBuffer == nullptr) {
		GraphicsAPI::UniformBuffer::CreateInfo uniformBufferCreateInfo{};
		uniformBufferCreateInfo.debugName = "Per Draw Uniform Buffer";
		uniformBufferCreateInfo.isDynamic = true;
		uniformBufferCreateInfo.size = sizeof(float) * 16;
		meshRenderComponent.perDrawUniformBuffer = graphicsCore->CreateUniformBuffer(uniformBufferCreateInfo);

		GraphicsAPI::DescriptorSet::Binding descriptorSetUniformBinding{};
		descriptorSetUniformBinding.bindingIndex = 0;
		descriptorSetUniformBinding.bindingType = GraphicsAPI::BindingType::UniformBuffer;
		descriptorSetUniformBinding.count = 1;
		descriptorSetUniformBinding.itemPtr = meshRenderComponent.perDrawUniformBuffer;

		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Per Draw Descriptor Set";
		descriptorSetCreateInfo.bindingCount = 1;
		descriptorSetCreateInfo.bindings = &descriptorSetUniformBinding;
		descriptorSetCreateInfo.layout = perDrawDescriptorSetLayout;
		meshRenderComponent.perDrawDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}

void Mesh3dRenderer::RenderShadowMap(GraphicsAPI::CommandBuffer* commandBuffer, GraphicsAPI::DescriptorSet* lightingDescriptorSet, entt::registry& registry, glm::vec3 lightSourcePosition) {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();
	Assets::AssetManager* assetManager = engineCore->assetManager;

	auto skybox = renderQueueMap.find("Skybox");
	RenderQueueIndex renderQueueSkyIndex = INVALID_RENDER_QUEUE;
	if (skybox == renderQueueMap.end()) {
		renderQueueSkyIndex = skybox->second;
	}

	std::vector<RenderTask> renderTasks;
	std::vector<RenderSortData> renderSortData;

	auto view = registry.view<const entt::entity, const TransformComponent, const MeshComponent, MeshRendererComponent>();
	view.each(
		[&](
			const entt::entity entity,
			const TransformComponent& transformComponent,
			const MeshComponent& meshComponent,
			MeshRendererComponent& meshRenderComponent
		) {
		// Add to Render Queue
		Mesh3dAsset* meshAsset = assetManager->GetAsset(meshComponent.mesh);

		if (meshAsset == nullptr) {
			return;
		}

		ValidateMeshRenderer(meshRenderComponent);

		// Early Frustum Cull

		Math::Matrix4 transform = TransformComponent::GetWorldTransformMatrix(entity, registry);
		meshRenderComponent.perDrawUniformBuffer->UpdateBuffer(&transform);

		glm::vec3 offset = TransformComponent::GetWorldPosition(entity, registry) - lightSourcePosition;
		// Multiply because this will be simplified to an int.
		const float distanceMultiplier = 100.0f;
		// Distance squared is used to sort. It's squared because we don't need exact distance to sort, and it's an expensive function.
		float distanceSquared = (offset.x * offset.x) + (offset.y * offset.y) + (offset.z * offset.z);
		distanceSquared *= distanceMultiplier;
		uint32_t sortData = static_cast<uint32_t>(distanceSquared);

		const std::vector<Grindstone::AssetReference<Grindstone::MaterialAsset>>& materials = meshRenderComponent.materials;
		for (const Grindstone::Mesh3dAsset::Submesh& submesh : meshAsset->submeshes) {
			if (submesh.materialIndex >= materials.size()) {
				continue;
			}

			const Grindstone::AssetReference<Grindstone::MaterialAsset> materialAssetReference = materials[submesh.materialIndex];
			if (!materialAssetReference.uuid.IsValid()) {
				continue;
			}

			MaterialAsset* materialAsset = assetManager->GetAsset(materialAssetReference);
			if (materialAsset == nullptr) {
				continue;
			}

			ShaderAsset* shaderAsset = assetManager->GetAsset<ShaderAsset>(materialAsset->shaderUuid);
			if (shaderAsset == nullptr) {
				continue;
			}

			RenderTask renderTask{};
			renderTask.materialDescriptorSet = materialAsset->descriptorSet;
			renderTask.pipeline = shaderAsset->pipeline;
			renderTask.vertexArrayObject = meshAsset->vertexArrayObject;
			renderTask.indexCount = submesh.indexCount;
			renderTask.baseVertex = submesh.baseVertex;
			renderTask.baseIndex = submesh.baseIndex;
			renderTask.perDrawDescriptorSet = meshRenderComponent.perDrawDescriptorSet;
			renderTask.sortData = sortData;
			renderTasks.emplace_back(renderTask);
		}
	});

	renderSortData.resize(renderTasks.size());
	for (size_t i = 0; i < renderSortData.size(); ++i) {
		renderSortData[i].renderTaskIndex = i;
		renderSortData[i].sortData = renderTasks[i].sortData;
	}

	std::sort(renderSortData.begin(), renderSortData.end(), CompareRenderSort);

	for (size_t sortIndex = 0; sortIndex < renderSortData.size(); ++sortIndex) {
		size_t renderTaskIndex = renderSortData[sortIndex].renderTaskIndex;
		Grindstone::RenderTask& renderTask = renderTasks[renderTaskIndex];

		commandBuffer->BindVertexArrayObject(renderTask.vertexArrayObject);

		commandBuffer->DrawIndices(
			renderTask.baseIndex,
			renderTask.indexCount,
			1,
			renderTask.baseVertex
		);
	}
}

void Mesh3dRenderer::RenderQueue(GraphicsAPI::CommandBuffer* commandBuffer, const char* queueName) {
	RenderQueueContainer& renderQueue = renderQueues[renderQueueMap[queueName]];

	GraphicsAPI::GraphicsPipeline* graphicsPipeline = nullptr;
	Assets::AssetManager* assetManager = engineCore->assetManager;

	for (size_t sortIndex = 0; sortIndex < renderQueue.renderSortData.size(); ++sortIndex) {
		size_t renderTaskIndex = renderQueue.renderSortData[sortIndex].renderTaskIndex;
		Grindstone::RenderTask& renderTask = renderQueue.renderTasks[renderTaskIndex];
		if (graphicsPipeline != renderTask.pipeline) {
			graphicsPipeline = renderTask.pipeline;
			commandBuffer->BindGraphicsPipeline(renderTask.pipeline);
		}

		commandBuffer->BindVertexArrayObject(renderTask.vertexArrayObject);
		std::array<GraphicsAPI::DescriptorSet*, 3> descriptors = { renderTask.materialDescriptorSet, engineDescriptorSet, renderTask.perDrawDescriptorSet };
		commandBuffer->BindGraphicsDescriptorSet(
			graphicsPipeline,
			descriptors.data(),
			static_cast<uint32_t>(descriptors.size())
		);

		commandBuffer->DrawIndices(
			renderTask.baseIndex,
			renderTask.indexCount,
			1,
			renderTask.baseVertex
		);
	}
}

void Mesh3dRenderer::CacheRenderTasksAndFrustumCull(glm::vec3 eyePosition, entt::registry& registry) {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();
	Assets::AssetManager* assetManager = engineCore->assetManager;

	for(auto& renderQueue : renderQueues) {
		renderQueue.renderTasks.clear();
		renderQueue.renderTasks.reserve(8000);
	}

	auto view = registry.view<const entt::entity, const TransformComponent, const MeshComponent, MeshRendererComponent>();
	view.each([&](
		const entt::entity entity,
		const TransformComponent& transformComponent,
		const MeshComponent& meshComponent,
		MeshRendererComponent& meshRenderComponent
	) {
		// Add to Render Queue
		Mesh3dAsset* meshAsset = assetManager->GetAsset(meshComponent.mesh);

		if (meshAsset == nullptr) {
			return;
		}

		ValidateMeshRenderer(meshRenderComponent);

		// Early Frustum Cull

		Math::Matrix4 transform = TransformComponent::GetWorldTransformMatrix(entity, registry);
		meshRenderComponent.perDrawUniformBuffer->UpdateBuffer(&transform);

		glm::vec3 offset = TransformComponent::GetWorldPosition(entity, registry) - eyePosition;
		// Multiply because this will be simplified to an int.
		const float distanceMultiplier = 100.0f;
		// Distance squared is used to sort. It's squared because we don't need exact distance to sort, and it's an expensive function.
		float distanceSquared = (offset.x * offset.x) + (offset.y * offset.y) + (offset.z * offset.z);
		distanceSquared *= distanceMultiplier;
		uint32_t sortData = static_cast<uint32_t>(distanceSquared);

		const std::vector<Grindstone::AssetReference<Grindstone::MaterialAsset>>& materials = meshRenderComponent.materials;
		for (const Grindstone::Mesh3dAsset::Submesh& submesh : meshAsset->submeshes) {
			if (submesh.materialIndex >= materials.size()) {
				continue;
			}

			MaterialAsset* materialAsset = assetManager->GetAsset(materials[submesh.materialIndex]);
			if (materialAsset == nullptr) {
				continue;
			}

			ShaderAsset* shaderAsset = assetManager->GetAsset<ShaderAsset>(materialAsset->shaderUuid);
			if (shaderAsset == nullptr) {
				continue;
			}

			RenderTask renderTask{};
			renderTask.materialDescriptorSet = materialAsset->descriptorSet;
			renderTask.pipeline = shaderAsset->pipeline;
			renderTask.vertexArrayObject = meshAsset->vertexArrayObject;
			renderTask.indexCount = submesh.indexCount;
			renderTask.baseVertex = submesh.baseVertex;
			renderTask.baseIndex = submesh.baseIndex;
			renderTask.perDrawDescriptorSet = meshRenderComponent.perDrawDescriptorSet;
			renderTask.sortData = sortData;

			RenderQueueIndex renderQueueIndex = shaderAsset->renderQueue;

			if (renderQueueIndex == INVALID_RENDER_QUEUE || renderQueueIndex >= renderQueues.size()) {
				continue;
			}

			renderQueues[renderQueueIndex].renderTasks.emplace_back(renderTask);
		}
	});
}

void Mesh3dRenderer::SortQueues() {
	for (RenderQueueContainer& renderQueue : renderQueues) {
		std::vector<RenderSortData>& renderSortData = renderQueue.renderSortData;
		RenderTaskList& renderTasks = renderQueue.renderTasks;

		renderSortData.resize(renderTasks.size());

		// Build sort data list
		for (size_t i = 0; i < renderSortData.size(); ++i) {
			renderSortData[i].renderTaskIndex = i;
			renderSortData[i].sortData = renderTasks[i].sortData;
		}

		// Sort the list
		if (renderQueue.sortMode == DrawSortMode::DistanceFrontToBack) {
			std::sort(renderSortData.begin(), renderSortData.end(), CompareRenderSort);
		}
		else {
			std::sort(renderSortData.begin(), renderSortData.end(), CompareReverseRenderSort);
		}
	}
}
