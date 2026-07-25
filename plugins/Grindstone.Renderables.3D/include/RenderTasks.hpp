#pragma once

#include <Common/HashedString.hpp>
#include <Common/Rendering/GeometryRenderingStats.hpp>
#include <Grindstone.Renderables.3D/include/Assets/Mesh3dAsset.hpp>
#include <Grindstone.Renderables.3D/include/FrustumCulling.hpp>
#include <Grindstone.Renderables.3D/include/Components/MeshRendererComponent.hpp>

namespace Grindstone::Renderer {
	struct RenderableBufferPair {
		glm::mat4 matrix;
		uint32_t entityId;
	};

	template<typename MeshComponentType, typename RenderTask>
	std::vector<RenderTask> GenerateTaskList(
		Grindstone::Rendering::GeometryRenderStats& renderingStats,
		entt::registry& registry,
		const Grindstone::Renderer::CullingFrustum& frustum,
		const glm::mat4& viewMatrix,
		Grindstone::HashedString renderQueueHash,
		std::function<void(
			std::vector<RenderTask>&,
			const Grindstone::HashedString,
			const Mesh3dAsset::Submesh&,
			const Mesh3dAsset*,
			const MeshComponentType&,
			const MeshRendererComponent&
		)> submeshCallback
	) {
		std::vector<RenderTask> renderTasks;
		renderTasks.reserve(1000);

		auto view = registry.view<const entt::entity, const TransformComponent, const MeshComponentType, MeshRendererComponent>();
		view.each(
			[&submeshCallback, &renderingStats, &registry, &renderTasks, &viewMatrix, frustum, renderQueueHash](
				entt::entity entity,
				const TransformComponent& transformComponent,
				const MeshComponentType& meshComponent,
				MeshRendererComponent& meshRenderComponent
			) {
				const Mesh3dAsset* meshAsset = meshComponent.mesh.Get();

				if (meshAsset == nullptr) {
					return;
				}

				Math::Matrix4 transform = TransformComponent::GetWorldTransformMatrix(entity, registry);
				glm::mat4 viewTransformTransform = viewMatrix * transform;

				Grindstone::Renderer::AABB aabb{ meshAsset->boundingData.minAABB, meshAsset->boundingData.maxAABB };
				if (!IsInFrustum(frustum, viewTransformTransform, aabb)) {
					renderingStats.objectsCulled += 1;
					return;
				}

				renderingStats.objectsRendered += 1;

				RenderableBufferPair renderableData{
					.matrix = transform,
					.entityId = static_cast<uint32_t>(entity)
				};
				meshRenderComponent.perDrawUniformBuffer->UploadData(&renderableData);

				std::vector<Grindstone::AssetReference<Grindstone::MaterialAsset>>& materials = meshRenderComponent.materials;
				for (const Grindstone::Mesh3dAsset::Submesh& submesh : meshAsset->submeshes) {
					submeshCallback(
						renderTasks,
						renderQueueHash,
						submesh,
						meshAsset,
						meshComponent,
						meshRenderComponent
					);
				}
			}
		);

		return renderTasks;
	}

	template<typename RenderTask>
	void RenderAllTasks(
		Grindstone::Rendering::GeometryRenderStats& renderingStats,
		GraphicsAPI::DescriptorSet* engineDescriptorSet,
		GraphicsAPI::CommandBuffer* commandBuffer,
		const std::vector<RenderTask>& renderTasks
	) {
		const GraphicsAPI::PipelineLayout* pipelineLayout = nullptr;
		const GraphicsAPI::GraphicsPipeline* graphicsPipeline = nullptr;
		const GraphicsAPI::DescriptorSet* materialDescriptorSet = nullptr;

		for (const RenderTask& renderTask : renderTasks) {
			if (graphicsPipeline != renderTask.pipeline) {
				graphicsPipeline = renderTask.pipeline;
				pipelineLayout = graphicsPipeline->pipelineLayout;
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
					pipelineLayout,
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
	}
}
