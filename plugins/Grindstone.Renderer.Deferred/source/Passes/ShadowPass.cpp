#include <EngineCore/Scenes/Manager.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/CoreComponents/Lights/DirectionalLightComponent.hpp>
#include <EngineCore/CoreComponents/Lights/PointLightComponent.hpp>
#include <EngineCore/CoreComponents/Lights/SpotLightComponent.hpp>

#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/ShadowPass.hpp>

static void RenderShadowMap(
	Grindstone::WorldContextSet* cxtSet,
	Grindstone::Rendering::RenderViewData renderViewData,
	Grindstone::EngineCore& engineCore,
	Grindstone::GraphicsAPI::CommandBuffer* cmd
) {
	cmd->SetViewport(0.0f, 0.0f, static_cast<float>(renderViewData.renderArea.GetWidth()), static_cast<float>(renderViewData.renderArea.GetHeight()));
	cmd->SetScissor(0, 0, renderViewData.renderArea.GetWidth(), renderViewData.renderArea.GetHeight());

	// TODO: Get Rendering Stats
	engineCore.assetRendererManager->RenderQueue(cmd, renderViewData, cxtSet->GetEntityRegistry(), shadowMapRenderPassKey);
}

static void RenderSpotLightComponent(
	Grindstone::Math::IntRect2D renderArea,
	Grindstone::GraphicsAPI::CommandBuffer* cmd,
	Grindstone::WorldContextSet* cxtSet, 
	const ECS::Entity entity,
	Grindstone::SpotLightComponent& spotLightComponent
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
	engineCore.assetRendererManager->SetEngineDescriptorSet(spotLightComponent.shadowMapDescriptorSet);

	Grindstone::Rendering::RenderViewData renderViewData{
		.projectionMatrix = projectionMatrix,
		.viewMatrix = viewMatrix,
		.renderArea = renderArea
	};

	RenderShadowMap(cxtSet, renderViewData, engineCore, cmd);
}

static void RenderDirectionalLightComponent(
	Grindstone::Math::IntRect2D renderArea,
	Grindstone::GraphicsAPI::CommandBuffer* cmd,
	Grindstone::WorldContextSet* cxtSet,
	const ECS::Entity entity,
	Grindstone::DirectionalLightComponent& directionalLightComponent
) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	float aspectRatio = 1.0f;
	float nearDist = 0.1f;

	const glm::vec3 forwardVector = entity.GetWorldForward();
	const glm::vec3 pos = entity.GetWorldPosition();

	const float shadowHalfSize = 40.0f;
	glm::mat4 projectionMatrix = glm::ortho<float>(-shadowHalfSize, shadowHalfSize, -shadowHalfSize, shadowHalfSize, 0, 160.0f);
	engineCore.GetGraphicsCore()->AdjustPerspective(&projectionMatrix[0][0]);

	Math::Float3 forward = entity.GetWorldForward();

	// TODO: Calculate bounds of camera frustum and use it here.
	glm::vec3 lightPos = forward * -100.0f;
	const glm::mat4 viewMatrix = glm::lookAt(
		lightPos,
		glm::vec3(0, 0, 0),
		entity.GetWorldUp()
	);

	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

	directionalLightComponent.shadowMatrix = projectionMatrix * viewMatrix;
	directionalLightComponent.shadowMapUniformBufferObject->UploadData(&directionalLightComponent.shadowMatrix);
	engineCore.assetRendererManager->SetEngineDescriptorSet(directionalLightComponent.shadowMapDescriptorSet);

	Grindstone::Rendering::RenderViewData renderViewData{
		.projectionMatrix = projectionMatrix,
		.viewMatrix = viewMatrix,
		.renderArea = renderArea
	};

	RenderShadowMap(cxtSet, renderViewData, engineCore, cmd);
}

bool Grindstone::Renderer::ShadowPass::Initialize() {
	return true;
}

void Grindstone::Renderer::ShadowPass::AddPass(Grindstone::Renderer::RenderGraph& renderGraph) {
	renderGraph.AddGraphicsPass(
		"Shadow"_hash,
		[](Renderer::RenderGraph::RenderPass& renderPass) {
			renderPass.WriteDepthStencilAttachment(attachmentNameShadowDepthStencil, attachmentShadowDepthStencil, Grindstone::GraphicsAPI::ClearDepthStencil(1.0f, 0u));
		},
		[this](const Renderer::RenderGraph::RenderGraphContext& cxt, Renderer::RenderGraph::RenderPassExecution& renderPassExecution) {
			Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
			Grindstone::WorldContextSet* cxtSet = cxt.worldContextSet;
			Grindstone::GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;
			entt::registry& registry = cxtSet->GetEntityRegistry();

			Grindstone::SceneManagement::Scene* scene = engineCore.GetSceneManager()->scenes.begin()->second;

			// TODO: Point Light Shadows

			uint32_t totalShadowMapCount = 0;

			auto spotLightView = registry.view<const entt::entity, Grindstone::SpotLightComponent>();
			auto directionalLightView = registry.view<const entt::entity, Grindstone::DirectionalLightComponent>();

			spotLightView.each(
				[&totalShadowMapCount](const entt::entity entityHandle, Grindstone::SpotLightComponent& spotLightComponent) {
					++totalShadowMapCount;
				}
			);

			directionalLightView.each(
				[&totalShadowMapCount](const entt::entity entityHandle, Grindstone::DirectionalLightComponent& directionalLightComponent) {
					++totalShadowMapCount;
				}
			);

			PrepareAtlas(totalShadowMapCount);

			spotLightView.each(
				[this, &cxtSet, cmd, scene](const entt::entity entityHandle, Grindstone::SpotLightComponent& spotLightComponent) {
					const ECS::Entity entity = ECS::Entity(entityHandle, scene);
					Grindstone::Math::IntRect2D renderArea;
					if (GetAtlasRenderArea(renderArea)) {
						RenderSpotLightComponent(renderArea, cmd, cxtSet, entity, spotLightComponent);
					}
				}
			);

			directionalLightView.each(
				[this, &cxtSet, cmd, scene](const entt::entity entityHandle, DirectionalLightComponent& directionalLightComponent) {
					const ECS::Entity entity = ECS::Entity(entityHandle, scene);
					Grindstone::Math::IntRect2D renderArea;
					if (GetAtlasRenderArea(renderArea)) {
						RenderDirectionalLightComponent(renderArea, cmd, cxtSet, entity, directionalLightComponent);
					}
				}
			);
		}
	);
}

void Grindstone::Renderer::ShadowPass::PrepareAtlas(uint32_t totalShadowMapCount) {
	currentAtlasIndex = 0;

	maxAtlasCount = 1;
	shadowResolution = shadowAtlasResolution;

	for (uint32_t i = 1; i < 16; ++i) {
		maxAtlasCount = i * i;
		shadowResolution /= 2;

		if (maxAtlasCount >= totalShadowMapCount) {
			break;
		}
	}
}

bool Grindstone::Renderer::ShadowPass::GetAtlasRenderArea(Grindstone::Math::IntRect2D& rect) {
	++currentAtlasIndex;

	if (currentAtlasIndex > maxAtlasCount) {
		GS_ASSERT_ENGINE_WITH_MESSAGE(false, "Too many shadows! {} found, {} maximum expected.", currentAtlasIndex, maxAtlasCount);
		return false;
	}

	int32_t x = (currentAtlasIndex * shadowResolution) % shadowAtlasResolution;
	int32_t y = (currentAtlasIndex * shadowResolution) / shadowAtlasResolution;

	rect = Grindstone::Math::IntRect2D(x, y, shadowResolution, shadowResolution);
	return true;
}
