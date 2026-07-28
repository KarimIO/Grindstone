#include <EngineCore/Scenes/Manager.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/CoreComponents/Lights/DirectionalLightComponent.hpp>
#include <EngineCore/CoreComponents/Lights/PointLightComponent.hpp>
#include <EngineCore/CoreComponents/Lights/SpotLightComponent.hpp>
#include <EngineCore/CoreComponents/Tag/TagComponent.hpp>

#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/ShadowPass.hpp>

static Grindstone::Rendering::GeometryRenderStats RenderShadowMap(
	const std::string& passName,
	Grindstone::WorldContextSet* cxtSet,
	Grindstone::Rendering::RenderViewData renderViewData,
	Grindstone::EngineCore& engineCore,
	Grindstone::GraphicsAPI::CommandBuffer* cmd
) {
	Grindstone::GraphicsAPI::ClearAttachment clearAttachment{
		.aspectMask = GraphicsAPI::ImageAspectBits::Depth,
		.clearValue = GraphicsAPI::ClearDepthStencil(1.0f, 0u)
	};

	Grindstone::GraphicsAPI::ClearRect clearRect{
		.rect = renderViewData.renderArea,
		.baseArrayLayer = 0u,
		.layerCount = 1u
	};

	cmd->ClearAttachments(&clearAttachment, 1u, &clearRect, 1u);

	return engineCore.assetRendererManager->RenderQueue(passName, cmd, renderViewData, cxtSet->GetEntityRegistry(), shadowMapRenderPassKey);
}

static void RenderPointLightComponent(
	const std::string& passName,
	Grindstone::Math::IntRect2D renderArea,
	Grindstone::GraphicsAPI::CommandBuffer* cmd,
	Grindstone::WorldContextSet* cxtSet,
	const ECS::Entity entity,
	Grindstone::PointLightComponent& pointLightComponent,
	std::function<void(const Grindstone::Rendering::GeometryRenderStats&)> pushRenderingStatsCallback,
	size_t faceIndex
) {
	struct DirectionPair {
		glm::vec3 forward;
		glm::vec3 up;
	};

	const std::array<DirectionPair, 6> directions{
		DirectionPair{ glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0) },
		DirectionPair{ glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0) },
		DirectionPair{ glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0) },
		DirectionPair{ glm::vec3(0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0) },
		DirectionPair{ glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0) },
		DirectionPair{ glm::vec3(0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0) }
	};

	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	const static float fov = glm::radians(90.0f);
	const static float aspectRatio = 1.0f;
	const static float nearDist = 0.1f;
	const float farDist = pointLightComponent.attenuationRadius;

	const glm::vec3 pos = entity.GetWorldPosition();

	glm::mat4 projectionMatrix = glm::perspective(fov, aspectRatio, nearDist, farDist);
	const glm::mat4 viewMatrix = glm::lookAt(pos, pos + directions[faceIndex].forward, directions[faceIndex].up);

	engineCore.GetGraphicsCore()->AdjustPerspective(&projectionMatrix[0][0]);

	const glm::mat4 projView = projectionMatrix * viewMatrix;
	pointLightComponent.shadowData[faceIndex].shadowMatrix = projView;
	pointLightComponent.shadowMapUniformBufferObjects[faceIndex]->UploadData(&projView);
	float shadowAtlasResolutionF = static_cast<float>(shadowAtlasResolution);
	pointLightComponent.shadowData[faceIndex].shadowRenderArea = Math::Rect2D(
		static_cast<float>(renderArea.offset.x) / shadowAtlasResolutionF,
		static_cast<float>(renderArea.offset.y) / shadowAtlasResolutionF,
		static_cast<float>(renderArea.extent.x) / shadowAtlasResolutionF,
		static_cast<float>(renderArea.extent.y) / shadowAtlasResolutionF
	);
	engineCore.assetRendererManager->SetEngineDescriptorSet(pointLightComponent.shadowMapDescriptorSets[faceIndex]);

	Grindstone::Rendering::RenderViewData renderViewData{
		.projectionMatrix = projectionMatrix,
		.viewMatrix = viewMatrix,
		.renderArea = renderArea
	};

	pushRenderingStatsCallback(RenderShadowMap(passName, cxtSet, renderViewData, engineCore, cmd));
}

static void RenderSpotLightComponent(
	const std::string& passName,
	Grindstone::Math::IntRect2D renderArea,
	Grindstone::GraphicsAPI::CommandBuffer* cmd,
	Grindstone::WorldContextSet* cxtSet, 
	const ECS::Entity entity,
	Grindstone::SpotLightComponent& spotLightComponent,
	std::function<void(const Grindstone::Rendering::GeometryRenderStats&)> pushRenderingStatsCallback
) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	float fov = glm::radians(spotLightComponent.outerAngle * 2.0f);
	float aspectRatio = 1.0f;
	float nearDist = 0.1f;
	float farDist = spotLightComponent.attenuationRadius;

	const glm::vec3 forwardVector = entity.GetWorldForward();
	const glm::vec3 pos = entity.GetWorldPosition();

	glm::mat4 projectionMatrix = glm::perspective(fov, aspectRatio, nearDist, farDist);
	const glm::mat4 viewMatrix = glm::lookAt(pos, pos + forwardVector, entity.GetWorldUp());

	engineCore.GetGraphicsCore()->AdjustPerspective(&projectionMatrix[0][0]);

	spotLightComponent.shadowMatrix = projectionMatrix * viewMatrix;
	spotLightComponent.shadowMapUniformBufferObject->UploadData(&spotLightComponent.shadowMatrix);
	float shadowAtlasResolutionF = static_cast<float>(shadowAtlasResolution);
	spotLightComponent.shadowRenderArea = Math::Rect2D(
		static_cast<float>(renderArea.offset.x) / shadowAtlasResolutionF,
		static_cast<float>(renderArea.offset.y) / shadowAtlasResolutionF,
		static_cast<float>(renderArea.extent.x) / shadowAtlasResolutionF,
		static_cast<float>(renderArea.extent.y) / shadowAtlasResolutionF
	);
	engineCore.assetRendererManager->SetEngineDescriptorSet(spotLightComponent.shadowMapDescriptorSet);

	Grindstone::Rendering::RenderViewData renderViewData{
		.projectionMatrix = projectionMatrix,
		.viewMatrix = viewMatrix,
		.renderArea = renderArea
	};

	pushRenderingStatsCallback(RenderShadowMap(passName, cxtSet, renderViewData, engineCore, cmd));
}

static void RenderDirectionalLightComponent(
	const std::string& passName,
	Grindstone::Math::IntRect2D renderArea,
	Grindstone::GraphicsAPI::CommandBuffer* cmd,
	Grindstone::WorldContextSet* cxtSet,
	const ECS::Entity entity,
	Grindstone::DirectionalLightComponent& directionalLightComponent,
	std::function<void(const Grindstone::Rendering::GeometryRenderStats&)> pushRenderingStatsCallback
) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	float aspectRatio = 1.0f;
	float nearDist = 0.1f;

	const glm::vec3 forwardVector = entity.GetWorldForward();
	const glm::vec3 pos = entity.GetWorldPosition();

	const float shadowHalfSize = 10.0f;
	glm::mat4 projectionMatrix = glm::ortho<float>(-shadowHalfSize, shadowHalfSize, -shadowHalfSize, shadowHalfSize, 0, 160.0f);
	graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

	Math::Float3 forward = entity.GetWorldForward();

	// TODO: Calculate bounds of camera frustum and use it here.
	glm::vec3 lightPos = forward * -100.0f;
	const glm::mat4 viewMatrix = glm::lookAt(
		lightPos,
		glm::vec3(0, 0, 0),
		entity.GetWorldUp()
	);

	graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

	directionalLightComponent.shadowMatrix = projectionMatrix * viewMatrix;
	directionalLightComponent.shadowMapUniformBufferObject->UploadData(&directionalLightComponent.shadowMatrix);
	float shadowAtlasResolutionF = static_cast<float>(shadowAtlasResolution);
	directionalLightComponent.shadowRenderArea = Math::Rect2D(
		static_cast<float>(renderArea.offset.x) / shadowAtlasResolutionF,
		static_cast<float>(renderArea.offset.y) / shadowAtlasResolutionF,
		static_cast<float>(renderArea.extent.x) / shadowAtlasResolutionF,
		static_cast<float>(renderArea.extent.y) / shadowAtlasResolutionF
	);
	engineCore.assetRendererManager->SetEngineDescriptorSet(directionalLightComponent.shadowMapDescriptorSet);

	Grindstone::Rendering::RenderViewData renderViewData{
		.projectionMatrix = projectionMatrix,
		.viewMatrix = viewMatrix,
		.renderArea = renderArea
	};

	pushRenderingStatsCallback(RenderShadowMap(passName, cxtSet, renderViewData, engineCore, cmd));
}

static Grindstone::Renderer::RenderGraphBuilderResourceRef AddSpotShadowPass(
	Grindstone::Renderer::RenderGraphBuilder& renderGraph,
	const std::string& entityName,
	Grindstone::Renderer::RenderGraphBuilderResourceRef shadowAtlasRef,
	Grindstone::Renderer::MetaRect renderingArea,
	ECS::Entity entity,
	Grindstone::SpotLightComponent& spotLightComponent,
	std::function<void(const Grindstone::Rendering::GeometryRenderStats&)> pushRenderingStatsCallback
) {
	std::string passName = std::format("'{}' Shadow Pass Spot", entityName.c_str());
	return renderGraph.CreateGraphicsPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>(
		passName.c_str(),
		renderingArea,
		[shadowAtlasRef](Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>& renderPass) -> Grindstone::Renderer::RenderGraphBuilderResourceRef {
			Grindstone::Renderer::RenderGraphBuilderResourceRef ref = renderPass.WriteDepthStencilAttachment(
				shadowAtlasRef,
				GraphicsAPI::LoadOp::Load,
				Grindstone::GraphicsAPI::ClearDepthStencil(1.0f, 0u)
			);

			return ref;
		},
		[entity, &spotLightComponent, pushRenderingStatsCallback, passName](
			Grindstone::Math::IntRect2D viewportArea,
			const Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			Grindstone::Renderer::RenderGraphBuilderResourceRef& data
		) {
			RenderSpotLightComponent(
				passName,
				viewportArea,
				cxt.commandBuffer,
				cxt.worldContextSet,
				entity,
				spotLightComponent,
				pushRenderingStatsCallback
			);
		}
	);
}

static Grindstone::Renderer::RenderGraphBuilderResourceRef AddPointShadowPass(
	Grindstone::Renderer::RenderGraphBuilder& renderGraph,
	const std::string& entityName,
	Grindstone::Renderer::RenderGraphBuilderResourceRef shadowAtlasRef,
	Grindstone::Renderer::MetaRect renderingArea,
	ECS::Entity entity,
	Grindstone::PointLightComponent& pointLightComponent,
	std::function<void(const Grindstone::Rendering::GeometryRenderStats&)> pushRenderingStatsCallback,
	size_t faceIndex
) {
	std::string passName = std::format("'{}' Shadow Pass Point {}", entityName.c_str(), faceIndex);
	return renderGraph.CreateGraphicsPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>(
		passName.c_str(),
		renderingArea,
		[shadowAtlasRef](Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>& renderPass) -> Grindstone::Renderer::RenderGraphBuilderResourceRef {
			Grindstone::Renderer::RenderGraphBuilderResourceRef ref = renderPass.WriteDepthStencilAttachment(
				shadowAtlasRef,
				GraphicsAPI::LoadOp::Load,
				Grindstone::GraphicsAPI::ClearDepthStencil(1.0f, 0u)
			);

			return ref;
		},
		[entity, &pointLightComponent, faceIndex, pushRenderingStatsCallback, passName](
			Grindstone::Math::IntRect2D viewportArea,
			const Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			Grindstone::Renderer::RenderGraphBuilderResourceRef& data
		) {
			RenderPointLightComponent(
				passName,
				viewportArea,
				cxt.commandBuffer,
				cxt.worldContextSet,
				entity,
				pointLightComponent,
				pushRenderingStatsCallback,
				faceIndex
			);
		}
	);
}

static Grindstone::Renderer::RenderGraphBuilderResourceRef AddDirectionalShadowPass(
	Grindstone::Renderer::RenderGraphBuilder& renderGraph,
	const std::string& entityName,
	Grindstone::Renderer::RenderGraphBuilderResourceRef shadowAtlasRef,
	Grindstone::Renderer::MetaRect renderingArea,
	ECS::Entity entity,
	Grindstone::DirectionalLightComponent& directionalLight,
	std::function<void(const Grindstone::Rendering::GeometryRenderStats&)> pushRenderingStatsCallback
) {
	std::string passName = std::format("'{}' Shadow Pass Directional", entityName.c_str());
	return renderGraph.CreateGraphicsPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>(
		passName.c_str(),
		renderingArea,
		[shadowAtlasRef](Renderer::GraphicsRenderGraphBuilderPass<Grindstone::Renderer::RenderGraphBuilderResourceRef>& renderPass) -> Grindstone::Renderer::RenderGraphBuilderResourceRef {
			Grindstone::Renderer::RenderGraphBuilderResourceRef ref = renderPass.WriteDepthStencilAttachment(
				shadowAtlasRef,
				GraphicsAPI::LoadOp::Load,
				Grindstone::GraphicsAPI::ClearDepthStencil(1.0f, 0u)
			);

			return ref;
		},
		[entity, &directionalLight, pushRenderingStatsCallback, passName](
			Grindstone::Math::IntRect2D viewportArea,
			const Renderer::RenderGraphContext& cxt,
			const Grindstone::Renderer::RenderGraphFrameResources& frameResources,
			Grindstone::Renderer::RenderGraphBuilderResourceRef& data
		) {
			RenderDirectionalLightComponent(
				passName,
				viewportArea,
				cxt.commandBuffer,
				cxt.worldContextSet,
				entity,
				directionalLight,
				pushRenderingStatsCallback
			);
		}
	);
}

bool Grindstone::Renderer::ShadowPass::Initialize() {
	return true;
}

Grindstone::Renderer::ShadowPassReturnData Grindstone::Renderer::ShadowPass::AddShadowPasses(
	Grindstone::Renderer::RenderGraphBuilder& renderGraph,
	Grindstone::WorldContextSet& worldContextSet,
	std::function<void(const Grindstone::Rendering::GeometryRenderStats&)> pushRenderingStatsCallback
) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();

	Grindstone::SceneManagement::Scene* scene = engineCore.GetSceneManager()->scenes.begin()->second;

	uint32_t totalShadowMapCount = 0;

	entt::registry& registry = worldContextSet.GetEntityRegistry();
	Renderer::RenderGraphBuilderResourceRef shadowAtlasRef = renderGraph.AddImage(attachmentShadowDepthStencil, Renderer::invalidPassId);
	auto spotLightView = registry.view<const entt::entity, Grindstone::TagComponent, Grindstone::SpotLightComponent>();
	auto directionalLightView = registry.view<const entt::entity, Grindstone::TagComponent, Grindstone::DirectionalLightComponent>();
	auto pointLightView = registry.view<const entt::entity, Grindstone::TagComponent, Grindstone::PointLightComponent>();

	// I think reusing the view is faster than pulling a separate view without the tag component for this but I'm not sure.
	spotLightView.each(
		[&totalShadowMapCount](const entt::entity entityHandle, Grindstone::TagComponent&, const Grindstone::SpotLightComponent& spotLightComponent) {
			++totalShadowMapCount;
		}
	);

	directionalLightView.each(
		[&totalShadowMapCount](const entt::entity entityHandle, Grindstone::TagComponent&, const Grindstone::DirectionalLightComponent& directionalLightComponent) {
			++totalShadowMapCount;
		}
	);

	pointLightView.each(
		[&totalShadowMapCount](const entt::entity entityHandle, Grindstone::TagComponent&, const Grindstone::PointLightComponent& pointLightComponent) {
			totalShadowMapCount += 6;
		}
	);

	PrepareAtlas(totalShadowMapCount);

	spotLightView.each(
		[this, &renderGraph, &shadowAtlasRef, scene, pushRenderingStatsCallback](const entt::entity entityHandle, const Grindstone::TagComponent& tag, Grindstone::SpotLightComponent& spotLightComponent) {
			const ECS::Entity entity = ECS::Entity(entityHandle, scene);
			Grindstone::Math::IntRect2D renderArea;
			if (GetAtlasRenderArea(renderArea)) {
				Grindstone::Renderer::MetaRect metaRect = MetaRect::Pixels(
					static_cast<uint32_t>(renderArea.offset.x),
					static_cast<uint32_t>(renderArea.offset.y),
					renderArea.extent.x,
					renderArea.extent.y
				);
				shadowAtlasRef = AddSpotShadowPass(renderGraph, tag.tag, shadowAtlasRef, metaRect, entity, spotLightComponent, pushRenderingStatsCallback);
			}
		}
	);

	directionalLightView.each(
		[this, &renderGraph, &shadowAtlasRef, scene, pushRenderingStatsCallback](const entt::entity entityHandle, const Grindstone::TagComponent& tag, DirectionalLightComponent& directionalLightComponent) {
			const ECS::Entity entity = ECS::Entity(entityHandle, scene);
			Grindstone::Math::IntRect2D renderArea;
			if (GetAtlasRenderArea(renderArea)) {
				Grindstone::Renderer::MetaRect metaRect = MetaRect::Pixels(
					static_cast<uint32_t>(renderArea.offset.x),
					static_cast<uint32_t>(renderArea.offset.y),
					renderArea.extent.x,
					renderArea.extent.y
				);
				shadowAtlasRef = AddDirectionalShadowPass(renderGraph, tag.tag, shadowAtlasRef, metaRect, entity, directionalLightComponent, pushRenderingStatsCallback);
			}
		}
	);

	pointLightView.each(
		[this, &renderGraph, &shadowAtlasRef, scene, pushRenderingStatsCallback](const entt::entity entityHandle, const Grindstone::TagComponent& tag, Grindstone::PointLightComponent& pointLightComponent) {
			const ECS::Entity entity = ECS::Entity(entityHandle, scene);
			Grindstone::Math::IntRect2D renderArea;
			for (size_t i = 0; i < 6; ++i) {
				if (GetAtlasRenderArea(renderArea)) {
					Grindstone::Renderer::MetaRect metaRect = MetaRect::Pixels(
						static_cast<uint32_t>(renderArea.offset.x),
						static_cast<uint32_t>(renderArea.offset.y),
						renderArea.extent.x,
						renderArea.extent.y
					);
					shadowAtlasRef = AddPointShadowPass(renderGraph, tag.tag, shadowAtlasRef, metaRect, entity, pointLightComponent, pushRenderingStatsCallback, i);
				}
			}
		}
	);

	return { shadowAtlasRef };
}

void Grindstone::Renderer::ShadowPass::PrepareAtlas(uint32_t totalShadowMapCount) {
	currentAtlasIndex = 0;

	maxAtlasCount = 1;
	shadowResolution = shadowAtlasResolution;

	for (uint32_t i = 1; i < 16; i *= 2) {
		maxAtlasCount = i * i;
		if (maxAtlasCount >= totalShadowMapCount) {
			shadowResolution = shadowAtlasResolution / i;
			break;
		}
	}
}

bool Grindstone::Renderer::ShadowPass::GetAtlasRenderArea(Grindstone::Math::IntRect2D& rect) {
	if (currentAtlasIndex > maxAtlasCount) {
		GS_ASSERT_ENGINE_WITH_MESSAGE(false, "Too many shadows! {} found, {} maximum expected.", currentAtlasIndex, maxAtlasCount);
		return false;
	}

	int32_t x = (currentAtlasIndex * shadowResolution) % shadowAtlasResolution;
	int32_t y = shadowResolution * ((currentAtlasIndex * shadowResolution) / shadowAtlasResolution);

	rect = Grindstone::Math::IntRect2D(x, y, shadowResolution, shadowResolution);
	++currentAtlasIndex;

	return true;
}
