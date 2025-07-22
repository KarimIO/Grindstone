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

struct AABB {
	glm::vec3 center{ 0.f, 0.f, 0.f };
	glm::vec3 halfExtents{ 0.f, 0.f, 0.f };

	AABB(const glm::vec3& min, const glm::vec3& max) : center{ (max + min) * 0.5f }, halfExtents{ max.x - center.x, max.y - center.y, max.z - center.z } {}
	AABB(const glm::vec3& inCenter, float iI, float iJ, float iK) : center{ inCenter }, halfExtents{ iI, iJ, iK } {}
};

struct Plane {
	float a;
	float b;
	float c;
	float d;
};

struct Frustum {
	Plane planes[6];
};

static float GetSignedDistanceToPlane(const Plane& plane, const glm::vec3& point) {
	return glm::dot(glm::vec3(plane.a, plane.b, plane.c), point) - plane.d;
}

static bool IsOnOrForwardPlane(const AABB& aabb, const Plane& plane) {
	// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
	const float r =
		aabb.halfExtents.x * std::abs(plane.a) +
		aabb.halfExtents.y * std::abs(plane.b) +
		aabb.halfExtents.z * std::abs(plane.c);

	return -r <= GetSignedDistanceToPlane(plane, aabb.center);
}

static bool IsOnFrustum(const Frustum& camFrustum, const glm::mat4& worldTransformMatrix, const glm::vec3& center, const glm::vec3& halfExtents) {
	//Get global scale thanks to our transform
	const glm::vec3 globalCenter{ worldTransformMatrix * glm::vec4(center, 1.f) };
	const glm::quat rotation = Math::Quaternion(worldTransformMatrix);

	// Scaled orientation
	const glm::vec3 right = rotation * glm::vec3(1, 0, 0) * halfExtents.x;
	const glm::vec3 up = rotation * glm::vec3(0, 1, 0) * halfExtents.y;
	const glm::vec3 forward = rotation * glm::vec3(0, 0, 1) * halfExtents.z;

	const float newIi = std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, right)) +
		std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, up)) +
		std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, forward));

	const float newIj = std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, right)) +
		std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, up)) +
		std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, forward));

	const float newIk = std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, right)) +
		std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, up)) +
		std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, forward));

	const AABB worldAABB(globalCenter, newIi, newIj, newIk);

	return (
		IsOnOrForwardPlane(worldAABB, camFrustum.planes[0]) &&
		IsOnOrForwardPlane(worldAABB, camFrustum.planes[1]) &&
		IsOnOrForwardPlane(worldAABB, camFrustum.planes[2]) &&
		IsOnOrForwardPlane(worldAABB, camFrustum.planes[3]) &&
		IsOnOrForwardPlane(worldAABB, camFrustum.planes[4]) &&
		IsOnOrForwardPlane(worldAABB, camFrustum.planes[5])
	);
};

static Frustum CreateFrustumFromCamera(const glm::mat4& matrix) {
	Frustum frustum{};

	frustum.planes[0] = { // leftFace
		matrix[0][3] + matrix[0][0],
		matrix[1][3] + matrix[1][0],
		matrix[2][3] + matrix[2][0],
		matrix[3][3] + matrix[3][0]
	};

	frustum.planes[1] = { // rightFace
		matrix[0][3] - matrix[0][0],
		matrix[1][3] - matrix[1][0],
		matrix[2][3] - matrix[2][0],
		matrix[3][3] - matrix[3][0]
	};

	frustum.planes[2] = { // topFace
		matrix[0][3] - matrix[0][1],
		matrix[1][3] - matrix[1][1],
		matrix[2][3] - matrix[2][1],
		matrix[3][3] - matrix[3][1]
	};

	frustum.planes[3] = { // bottomFace
		matrix[0][3] + matrix[0][1],
		matrix[1][3] + matrix[1][1],
		matrix[2][3] + matrix[2][1],
		matrix[3][3] + matrix[3][1]
	};

	frustum.planes[4] = { // farFace
		matrix[0][3] - matrix[0][2],
		matrix[1][3] - matrix[1][2],
		matrix[2][3] - matrix[2][2],
		matrix[3][3] - matrix[3][2]
	};

	frustum.planes[5] = { // nearFace
		matrix[0][3] + matrix[0][2],
		matrix[1][3] + matrix[1][2],
		matrix[2][3] + matrix[2][2],
		matrix[3][3] + matrix[3][2]
	};

	for (int i = 0; i < 6; ++i) {
		float length = glm::length(
			glm::vec3{
				frustum.planes[i].a +
				frustum.planes[i].b +
				frustum.planes[i].c
			}
		);

		frustum.planes[i].a /= length;
		frustum.planes[i].b /= length;
		frustum.planes[i].c /= length;
		frustum.planes[i].d /= length;
	}

	return frustum;
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
	const Grindstone::Rendering::RenderViewData& renderViewData,
	entt::registry& registry,
	Grindstone::HashedString renderQueueHash
) {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();
	Assets::AssetManager* assetManager = engineCore->assetManager;

	std::vector<RenderTask> renderTasks;
	renderTasks.reserve(1000);

	Frustum frustum = CreateFrustumFromCamera(renderViewData.viewProjectionMatrix);

	auto view = registry.view<const entt::entity, const TransformComponent, const MeshComponent, MeshRendererComponent>();
	view.each(
		[&registry, &renderTasks, frustum, renderQueueHash, assetManager, graphicsCore](
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

			// Early Frustum Cull
			glm::vec3 aabbCenter = (meshAsset->boundingData.minAABB + meshAsset->boundingData.maxAABB) * 0.5f;
			glm::vec3 aabbHalfExtents = (meshAsset->boundingData.maxAABB - meshAsset->boundingData.minAABB) * 0.5f;
			if (!IsOnFrustum(frustum, transform, aabbCenter, aabbHalfExtents)) {
				return;
			}

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
