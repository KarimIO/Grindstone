#include <Grindstone.Renderer.Deferred/include/Passes/ShadowPass.hpp>

void RenderSpotShadowPass(const char* shadowPassName) {
}

bool Grindstone::Renderer::ShadowPass::Initialize() {
	return true;
}

void Grindstone::Renderer::ShadowPass::AddPass(Grindstone::Renderer::RenderGraph& renderGraph) {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	AssetRendererManager* assetManager = engineCore.assetRendererManager;
	SceneManagement::Scene* scene = engineCore.GetSceneManager()->scenes.begin()->second;

	GraphicsAPI::ClearDepthStencil clearDepthStencil{};
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;

	/* TODO: Finish with Point Light Shadows eventually
	{
		auto view = registry.view<const TransformComponent, PointLightComponent>();
		view.each([&](const TransformComponent& transformComponent, PointLightComponent& pointLightComponent) {
			float farDist = pointLightComponent.attenuationRadius;

			const glm::vec3 forwardVector = transformComponent.GetForward();
			const glm::vec3 pos = transformComponent.position;

			const auto viewMatrix = glm::lookAt(
				pos,
				pos + forwardVector,
				transformComponent.GetUp()
			);

			constexpr float fov = 90.0f;
			auto projectionMatrix = glm::perspective(
				fov,
				1.0f,
				0.1f,
				farDist
			);

			graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

			glm::mat4 shadowPass = projectionMatrix * viewMatrix * glm::scale(glm::mat4(1.0f), glm::vec3(0.02f));
			// pointLightComponent.shadowMatrix = projectionMatrix * viewMatrix * glm::mat4(1.0f);

			uint32_t resolution = static_cast<uint32_t>(pointLightComponent.shadowResolution);

			pointLightComponent.shadowMapUniformBufferObject->UploadData(&shadowPass);

			commandBuffer->BeginRendering(
				"Point Shadow Pass",
				pointLightComponent.framebuffer,
				resolution,
				resolution,
				nullptr,
				0,
				clearDepthStencil
			);

			commandBuffer->BindGraphicsPipeline(shadowMappingPipeline);

			float resF = static_cast<float>(resolution);
			commandBuffer->SetViewport(0.0f, 0.0f, resF, resF);
			commandBuffer->SetScissor(0, 0, resolution, resolution);

			commandBuffer->BindGraphicsDescriptorSet(shadowMappingPipeline, &pointLightComponent.shadowMapDescriptorSet, 0, 1);
			assetManager->RenderShadowMap(
				commandBuffer,
				spotLightComponent.shadowMapDescriptorSet,
				registry,
				transformComponent.position
			);

			commandBuffer->EndRendering();
		});
	}
	*/

	{
		auto view = registry.view<const entt::entity, SpotLightComponent>();
		view.each([&](const entt::entity entityHandle, SpotLightComponent& spotLightComponent) {
			const ECS::Entity entity = ECS::Entity(entityHandle, scene);

			float fov = glm::radians(spotLightComponent.outerAngle * 2.0f);
			float farDist = spotLightComponent.attenuationRadius;

			const glm::vec3 forwardVector = entity.GetWorldForward();
			const glm::vec3 pos = entity.GetWorldPosition();

			const auto viewMatrix = glm::lookAt(
				pos,
				pos + forwardVector,
				entity.GetWorldUp()
			);

			auto projectionMatrix = glm::perspective(
				fov,
				1.0f,
				0.1f,
				farDist
			);

			graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

			spotLightComponent.shadowMatrix = projectionMatrix * viewMatrix;

			if (spotLightComponent.shadowResolution != spotLightComponent.cachedShadowResolution) {
				graphicsCore->WaitUntilIdle();
				spotLightComponent.shadowResolution = std::clamp(spotLightComponent.shadowResolution, 8u, 16192u);
				spotLightComponent.cachedShadowResolution = spotLightComponent.shadowResolution;
				spotLightComponent.depthTarget->Resize(spotLightComponent.shadowResolution, spotLightComponent.shadowResolution);
				spotLightComponent.framebuffer->Resize(spotLightComponent.shadowResolution, spotLightComponent.shadowResolution);
				GraphicsAPI::DescriptorSet::Binding binding = GraphicsAPI::DescriptorSet::Binding::SampledImage(spotLightComponent.depthTarget);
				spotLightComponent.descriptorSet->ChangeBindings(&binding, 1, 1);
			}

			GraphicsAPI::ImageBarrier depthTargetBarrier = {
				.image = spotLightComponent.depthTarget,
				.oldLayout = Grindstone::GraphicsAPI::ImageLayout::Undefined,
				.newLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
				.srcAccess = Grindstone::GraphicsAPI::AccessFlags::None,
				.dstAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentWrite,
				.imageAspect = GraphicsAPI::ImageAspectBits::Depth,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			};

			commandBuffer->PipelineBarrier(
				GraphicsAPI::PipelineStageBit::TopOfPipe,
				GraphicsAPI::PipelineStageBit::EarlyFragmentTests,
				nullptr, 0,
				&depthTargetBarrier, 1u
			);

			uint32_t resolution = spotLightComponent.shadowResolution;

			spotLightComponent.shadowMapUniformBufferObject->UploadData(&spotLightComponent.shadowMatrix);
			assetManager->SetEngineDescriptorSet(spotLightComponent.shadowMapDescriptorSet);

			Grindstone::GraphicsAPI::RenderAttachment shadowRenderAttachment{
				.image = spotLightComponent.depthTarget,
				.imageLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
				.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
				.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
				.clearValue = clearDepthStencil
			};

			Grindstone::Math::IntRect2D shadowRenderArea = Grindstone::Math::IntRect2D(0u, 0u, resolution, resolution);

			commandBuffer->BeginRendering(
				"Spot Light Pass",
				shadowRenderArea,
				nullptr,
				0u,
				&shadowRenderAttachment,
				nullptr
			);

			Grindstone::Rendering::RenderViewData renderViewData{
				.projectionMatrix = projectionMatrix,
				.viewMatrix = viewMatrix,
				.renderArea = shadowRenderArea
			};

			commandBuffer->SetViewport(0.0f, 0.0f, static_cast<float>(resolution), static_cast<float>(resolution));
			commandBuffer->SetScissor(0, 0, resolution, resolution);
			assetManager->RenderQueue(
				commandBuffer,
				renderViewData,
				registry,
				shadowMapRenderPassKey
			);

			commandBuffer->EndRendering();
			});
	}

	{
		auto view = registry.view<const entt::entity, DirectionalLightComponent>();
		view.each([&](const entt::entity entityHandle, DirectionalLightComponent& directionalLightComponent) {
			const ECS::Entity entity = ECS::Entity(entityHandle, scene);

			const float shadowHalfSize = 40.0f;
			glm::mat4 projectionMatrix = glm::ortho<float>(-shadowHalfSize, shadowHalfSize, -shadowHalfSize, shadowHalfSize, 0, 160.0f);
			graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

			Math::Float3 forward = entity.GetWorldForward();

			glm::vec3 lightPos = forward * -100.0f;
			glm::mat4 viewMatrix = glm::lookAt(
				lightPos,
				glm::vec3(0, 0, 0),
				entity.GetWorldUp()
			);

			glm::mat4 projView = projectionMatrix * viewMatrix;
			directionalLightComponent.shadowMatrix = projView;

			if (directionalLightComponent.shadowResolution != directionalLightComponent.cachedShadowResolution) {
				graphicsCore->WaitUntilIdle();
				directionalLightComponent.shadowResolution = std::clamp(directionalLightComponent.shadowResolution, 8u, 16192u);
				directionalLightComponent.cachedShadowResolution = directionalLightComponent.shadowResolution;
				directionalLightComponent.depthTarget->Resize(directionalLightComponent.shadowResolution, directionalLightComponent.shadowResolution);
				directionalLightComponent.framebuffer->Resize(directionalLightComponent.shadowResolution, directionalLightComponent.shadowResolution);
				GraphicsAPI::DescriptorSet::Binding binding = GraphicsAPI::DescriptorSet::Binding::SampledImage(directionalLightComponent.depthTarget);
				directionalLightComponent.descriptorSet->ChangeBindings(&binding, 1, 1);
			}

			GraphicsAPI::ImageBarrier depthTargetBarrier = {
				.image = directionalLightComponent.depthTarget,
				.oldLayout = Grindstone::GraphicsAPI::ImageLayout::Undefined,
				.newLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
				.srcAccess = Grindstone::GraphicsAPI::AccessFlags::None,
				.dstAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentWrite,
				.imageAspect = GraphicsAPI::ImageAspectBits::Depth,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			};

			commandBuffer->PipelineBarrier(
				GraphicsAPI::PipelineStageBit::TopOfPipe,
				GraphicsAPI::PipelineStageBit::EarlyFragmentTests,
				nullptr, 0,
				&depthTargetBarrier, 1u
			);

			uint32_t resolution = directionalLightComponent.shadowResolution;

			directionalLightComponent.shadowMapUniformBufferObject->UploadData(&projView);
			assetManager->SetEngineDescriptorSet(directionalLightComponent.shadowMapDescriptorSet);

			Grindstone::GraphicsAPI::RenderAttachment shadowRenderAttachment{
				.image = directionalLightComponent.depthTarget,
				.imageLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
				.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
				.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
				.clearValue = clearDepthStencil
			};

			float resF = static_cast<float>(resolution);
			Grindstone::Math::IntRect2D shadowRenderArea = Grindstone::Math::IntRect2D(0u, 0u, resolution, resolution);

			commandBuffer->BeginRendering(
				"Directional Shadow Map Pass",
				shadowRenderArea,
				nullptr,
				0u,
				&shadowRenderAttachment,
				nullptr
			);

			Grindstone::Rendering::RenderViewData renderViewData{
				.projectionMatrix = projectionMatrix,
				.viewMatrix = viewMatrix,
				.renderArea = shadowRenderArea,
			};

			commandBuffer->SetViewport(0.0f, 0.0f, resF, resF);
			commandBuffer->SetScissor(0, 0, resolution, resolution);
			assetManager->RenderQueue(
				commandBuffer,
				renderViewData,
				registry,
				shadowMapRenderPassKey
			);

			commandBuffer->EndRendering();
		});
	}
}
