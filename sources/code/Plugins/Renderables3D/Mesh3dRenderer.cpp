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

struct CullingFrustum {
	float nearRight;
	float nearTop;
	float nearDistance;
	float farDistance;
};

struct AABB {
	glm::vec3 min;
	glm::vec3 max;
};

struct OBB {
	glm::vec3 center = {};
	glm::vec3 extents = {};
	glm::vec3 axes[3] = {};
};

// Thanks to Bruno Opsenica https://bruop.github.io/improved_frustum_culling/
static bool IsInFrustum(const CullingFrustum& frustum, const glm::mat4& viewModelMatrix, const AABB& aabb) {
	constexpr size_t cornerCount = 4;

	float z_near = frustum.nearDistance;
	float z_far = frustum.farDistance;

	float x_near = frustum.nearRight;
	float y_near = frustum.nearTop;

	// Consider four adjacent corners of the ABB
	glm::vec3 corners[] = {
		{aabb.min.x, aabb.min.y, aabb.min.z},
		{aabb.max.x, aabb.min.y, aabb.min.z},
		{aabb.min.x, aabb.max.y, aabb.min.z},
		{aabb.min.x, aabb.min.y, aabb.max.z},
	};

	// NOTE: Only works with affine matrices
	for (size_t corner_idx = 0; corner_idx < cornerCount; corner_idx++) {
		corners[corner_idx] = (viewModelMatrix * glm::vec4(corners[corner_idx], 1.0f));
	}

	OBB obb = {
		.axes = {
			corners[1] - corners[0],
			corners[2] - corners[0],
			corners[3] - corners[0]
		},
	};

	obb.center = corners[0] + 0.5f * (obb.axes[0] + obb.axes[1] + obb.axes[2]);
	obb.extents = glm::vec3{ length(obb.axes[0]), length(obb.axes[1]), length(obb.axes[2]) };
	obb.axes[0] = obb.axes[0] / obb.extents.x;
	obb.axes[1] = obb.axes[1] / obb.extents.y;
	obb.axes[2] = obb.axes[2] / obb.extents.z;
	obb.extents *= 0.5f;

	{
		glm::vec3 M = { 0.0f, 0.0f, 1.0f };
		float MoX = 0.0f;	// | m . x |
		float MoY = 0.0f;	// | m . y |
		float MoZ = M.z;	// m . z (not abs!)

		float MoC = obb.center.z;

		float radius = 0.0f;
		for (size_t i = 0; i < 3; i++) {
			radius += fabsf(obb.axes[i].z) * (&obb.extents.x)[i];
		}
		float obb_min = MoC - radius;
		float obb_max = MoC + radius;

		float m0 = z_far;
		float m1 = z_near;

		if (obb_min > m1 || obb_max < m0) {
			return false;
		}
	}

	{
		// Frustum normals
		const glm::vec3 M[] = {
			{ 0.0, -z_near, y_near },	// Top plane
			{ 0.0, z_near, y_near },	// Bottom plane
			{ -z_near, 0.0f, x_near },	// Right plane
			{ z_near, 0.0f, x_near },	// Left Plane
		};

		for (size_t m = 0; m < cornerCount; m++) {
			float MoX = fabsf(M[m].x);
			float MoY = fabsf(M[m].y);
			float MoZ = M[m].z;
			float MoC = dot(M[m], obb.center);

			float obb_radius = 0.0f;
			for (size_t i = 0; i < 3; i++) {
				obb_radius += fabsf(dot(M[m], obb.axes[i])) * (&obb.extents.x)[i];
			}
			float obb_min = MoC - obb_radius;
			float obb_max = MoC + obb_radius;

			float p = x_near * MoX + y_near * MoY;

			float tau_0 = z_near * MoZ - p;
			float tau_1 = z_near * MoZ + p;

			if (tau_0 < 0.0f) {
				tau_0 *= z_far / z_near;
			}
			if (tau_1 > 0.0f) {
				tau_1 *= z_far / z_near;
			}

			if (obb_min > tau_1 || obb_max < tau_0) {
				return false;
			}
		}
	}

	return true;
}

static CullingFrustum CreateFrustum(const Grindstone::Rendering::RenderViewData& renderViewData) {
	float aspectRatio = renderViewData.renderTargetSize.x / renderViewData.renderTargetSize.y;
	float tanFov = 1.0f / renderViewData.projectionMatrix[0][0];

	float projection_43 = renderViewData.projectionMatrix[3][2];
	float projection_33 = renderViewData.projectionMatrix[2][2];
	float nearDistance = projection_43 / (projection_33 - 1.0f);
	float farDistance = projection_43 / (projection_33 + 1.0f);

	return CullingFrustum{
		.nearRight = aspectRatio * nearDistance * tanFov,
		.nearTop = nearDistance * tanFov,
		.nearDistance = -nearDistance,
		.farDistance = -farDistance,
	};
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

Grindstone::Rendering::GeometryRenderStats Mesh3dRenderer::RenderQueue(
	GraphicsAPI::CommandBuffer* commandBuffer,
	const Grindstone::Rendering::RenderViewData& renderViewData,
	entt::registry& registry,
	Grindstone::HashedString renderQueueHash
) {
	Grindstone::Rendering::GeometryRenderStats renderingStats{};
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();
	Assets::AssetManager* assetManager = engineCore->assetManager;

	std::vector<RenderTask> renderTasks;
	renderTasks.reserve(1000);

	CullingFrustum frustum = CreateFrustum(renderViewData);

	glm::mat4 viewMatrix = renderViewData.viewMatrix;
	bool isOrtho = renderViewData.projectionMatrix[3][3] == 1.0f;

	std::chrono::time_point start = std::chrono::steady_clock::now();

	auto view = registry.view<const entt::entity, const TransformComponent, const MeshComponent, MeshRendererComponent>();
	view.each(
		[&renderingStats, &registry, &renderTasks, &viewMatrix, isOrtho, frustum, renderQueueHash, assetManager, graphicsCore](
			entt::entity entity,
			const TransformComponent& transformComponent,
			const MeshComponent& meshComponent,
			MeshRendererComponent& meshRenderComponent
		) {
			// Add to Render Queue
			Mesh3dAsset* meshAsset = assetManager->GetAssetByUuid<Mesh3dAsset>(meshComponent.mesh.uuid);

			if (meshAsset == nullptr) {
				return;
			}

			Math::Matrix4 transform = TransformComponent::GetWorldTransformMatrix(entity, registry);
			glm::mat4 viewTransformTransform = viewMatrix * transform;

			// TODO: Get Ortho culling working
			AABB aabb{ meshAsset->boundingData.minAABB, meshAsset->boundingData.maxAABB };
			if (!isOrtho && !IsInFrustum(frustum, viewTransformTransform, aabb)) {
				renderingStats.objectsCulled += 1;
				return;
			}

			renderingStats.objectsRendered += 1;

			meshRenderComponent.perDrawUniformBuffer->UploadData(&transform);

			std::vector<Grindstone::AssetReference<Grindstone::MaterialAsset>>& materials = meshRenderComponent.materials;
			for (const Grindstone::Mesh3dAsset::Submesh& submesh : meshAsset->submeshes) {
				if (submesh.materialIndex >= materials.size()) {
					continue;
				}

				Grindstone::AssetReference<Grindstone::MaterialAsset>& materialReference = materials[submesh.materialIndex];
				MaterialAsset* materialAsset = materialReference.Get();
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
	const GraphicsAPI::DescriptorSet* materialDescriptorSet = nullptr;

	for (RenderTask& renderTask : renderTasks) {
		if (graphicsPipeline != renderTask.pipeline) {
			graphicsPipeline = renderTask.pipeline;
			commandBuffer->BindGraphicsPipeline(renderTask.pipeline);
			renderingStats.pipelineBinds += 1;
		}

		commandBuffer->BindVertexArrayObject(renderTask.vertexArrayObject);

		if (renderTask.materialDescriptorSet != materialDescriptorSet) {
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
			renderingStats.materialBinds += 1;
		}

		renderingStats.drawCalls += 1;
		renderingStats.vertices += renderTask.indexCount;
		renderingStats.triangles += renderTask.indexCount / 3;

		commandBuffer->DrawIndices(
			renderTask.baseIndex,
			renderTask.indexCount,
			0,
			1,
			renderTask.baseVertex
		);
	}

	std::chrono::time_point end = std::chrono::steady_clock::now();
	long long ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	renderingStats.cpuTimeMs = static_cast<double>(ns) * 0.000001;

	return renderingStats;
}

GraphicsAPI::DescriptorSetLayout* Mesh3dRenderer::GetPerDrawDescriptorSetLayout() const {
	return perDrawDescriptorSetLayout;
}
