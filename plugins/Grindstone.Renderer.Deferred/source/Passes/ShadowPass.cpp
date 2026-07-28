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

static std::array<glm::vec4, 8> GetFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view) {
	const auto inv = glm::inverse(proj * view);

	std::array<glm::vec4, 8> frustumCorners{};
	for (uint32_t x = 0; x < 2; ++x) {
		for (uint32_t y = 0; y < 2; ++y) {
			for (uint32_t z = 0; z < 2; ++z) {
				const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, static_cast<float>(z), 1.0f);

				uint32_t index = z * 4 + y * 2 + x;
				frustumCorners[index] = (pt / pt.w);
			}
		}
	}

	return frustumCorners;
}

static std::tuple<glm::mat4, glm::mat4> GenerateCascadeMatrixFromCorners(
	const std::array<glm::vec4, 8>& cascadeFrustumCorners,
	const glm::vec3 cameraFrustumCenter,
	const glm::vec3 lightDir
) {
	glm::vec3 up = glm::vec3(0, 1, 0);

	if (abs(glm::dot(up, lightDir)) > 0.99f) {
		up = glm::vec3(0, 0, 1);
	}

	const glm::mat4 lightView = glm::lookAt(
		cameraFrustumCenter - lightDir,
		cameraFrustumCenter,
		up
	);

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();
	for (const glm::vec3& corner : cascadeFrustumCorners) {
		const glm::vec4 viewSpaceCorner = lightView * glm::vec4(corner, 1.0f);

		minX = std::min(minX, viewSpaceCorner.x);
		maxX = std::max(maxX, viewSpaceCorner.x);
		minY = std::min(minY, viewSpaceCorner.y);
		maxY = std::max(maxY, viewSpaceCorner.y);
		minZ = std::min(minZ, viewSpaceCorner.z);
		maxZ = std::max(maxZ, viewSpaceCorner.z);
	}

	float cascadeMaxDist = maxZ;

	constexpr float zMult = 100.0f;
	if (minZ < 0) {
		minZ *= zMult;
	}
	else {
		minZ /= zMult;
	}
	if (maxZ < 0) {
		maxZ /= zMult;
	}
	else {
		maxZ *= zMult;
	}

	glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
	Grindstone::EngineCore::GetInstance().GetGraphicsCore()->AdjustPerspective(&lightProjection[0][0]);

	return { lightProjection, lightView };
}

static std::array<glm::vec4, 8> GetCascadeFrustumCorners(const std::array<glm::vec4, 8>& cameraFrustumCorners, float cascadeNearFactor, float cascadeFarFactor) {
	const std::array<glm::vec4, 8> transformedCorners{
		glm::mix(cameraFrustumCorners[0], cameraFrustumCorners[4], cascadeNearFactor),
		glm::mix(cameraFrustumCorners[1], cameraFrustumCorners[5], cascadeNearFactor),
		glm::mix(cameraFrustumCorners[2], cameraFrustumCorners[6], cascadeNearFactor),
		glm::mix(cameraFrustumCorners[3], cameraFrustumCorners[7], cascadeNearFactor),
		glm::mix(cameraFrustumCorners[0], cameraFrustumCorners[4], cascadeFarFactor),
		glm::mix(cameraFrustumCorners[1], cameraFrustumCorners[5], cascadeFarFactor),
		glm::mix(cameraFrustumCorners[2], cameraFrustumCorners[6], cascadeFarFactor),
		glm::mix(cameraFrustumCorners[3], cameraFrustumCorners[7], cascadeFarFactor)
	};

	return transformedCorners;
}

static std::tuple<glm::mat4, glm::mat4> GenerateCascadeProjectionViewMatrix(
	const std::array<glm::vec4, 8>& cameraFrustumCorners,
	const glm::vec3 lightDirection,
	const float cascadeNearFactor,
	const float cascadeFarFactor
) {
	const std::array<glm::vec4, 8> cascadeFrustumCorners = GetCascadeFrustumCorners(cameraFrustumCorners, cascadeNearFactor, cascadeFarFactor);

	glm::vec3 cameraFrustumCenter = glm::vec3(0, 0, 0);
	for (const glm::vec4& v : cascadeFrustumCorners) {
		cameraFrustumCenter += glm::vec3(v);
	}
	cameraFrustumCenter /= cascadeFrustumCorners.size();

	return GenerateCascadeMatrixFromCorners(cascadeFrustumCorners, cameraFrustumCenter, lightDirection);
}

static void RenderDirectionalLightComponent(
	const std::string& passName,
	Grindstone::Math::IntRect2D renderArea,
	Grindstone::GraphicsAPI::CommandBuffer* cmd,
	Grindstone::WorldContextSet* cxtSet,
	const ECS::Entity entity,
	Grindstone::DirectionalLightComponent& directionalLightComponent,
	std::function<void(const Grindstone::Rendering::GeometryRenderStats&)> pushRenderingStatsCallback,
	const glm::mat4& lightProjectionMatrix,
	const glm::mat4& lightViewMatrix,
	uint32_t cascadeIndex
) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	directionalLightComponent.shadowData[cascadeIndex].shadowMatrix = lightProjectionMatrix * lightViewMatrix;
	directionalLightComponent.shadowMapUniformBufferObjects[cascadeIndex]->UploadData(&directionalLightComponent.shadowData[cascadeIndex].shadowMatrix);
	float shadowAtlasResolutionF = static_cast<float>(shadowAtlasResolution);
	directionalLightComponent.shadowData[cascadeIndex].shadowRenderArea = Math::Rect2D(
		static_cast<float>(renderArea.offset.x) / shadowAtlasResolutionF,
		static_cast<float>(renderArea.offset.y) / shadowAtlasResolutionF,
		static_cast<float>(renderArea.extent.x) / shadowAtlasResolutionF,
		static_cast<float>(renderArea.extent.y) / shadowAtlasResolutionF
	);
	engineCore.assetRendererManager->SetEngineDescriptorSet(directionalLightComponent.shadowMapDescriptorSets[cascadeIndex]);

	Grindstone::Rendering::RenderViewData renderViewData{
		.projectionMatrix = lightProjectionMatrix,
		.viewMatrix = lightViewMatrix,
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
	std::function<void(const Grindstone::Rendering::GeometryRenderStats&)> pushRenderingStatsCallback,
	const glm::mat4& lightProjectionMatrix,
	const glm::mat4& lightViewMatrix,
	uint32_t cascadeIndex
) {
	std::string passName = std::format("'{}' Shadow Pass Directional Cascade {}", entityName.c_str(), cascadeIndex);
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
		[entity, &directionalLight, lightProjectionMatrix, lightViewMatrix, pushRenderingStatsCallback, cascadeIndex, passName](
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
					pushRenderingStatsCallback,
					lightProjectionMatrix,
					lightViewMatrix,
					cascadeIndex
				);
		}
	);
}

static float GetCascadePlane(const uint32_t cascadeIndex, const uint32_t cascadeCount) {
	const float factor = static_cast<float>(cascadeIndex) / (cascadeCount + 1);
	return factor * factor;
}

bool Grindstone::Renderer::ShadowPass::Initialize() {
	return true;
}

Grindstone::Renderer::ShadowPassReturnData Grindstone::Renderer::ShadowPass::AddShadowPasses(
	const glm::mat4& cameraProjectionMatrix,
	const glm::mat4& cameraViewMatrix,
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
			totalShadowMapCount += directionalLightComponent.cascadeCount;
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

	const std::array<glm::vec4, 8> cameraFrustumCorners = GetFrustumCornersWorldSpace(cameraProjectionMatrix, cameraViewMatrix);

	float projection_43 = cameraProjectionMatrix[3][2];
	float projection_33 = cameraProjectionMatrix[2][2];
	float nearDistance = projection_43 / (projection_33 - 1.0f);
	float farDistance = projection_43 / (projection_33 + 1.0f);

	directionalLightView.each(
		[this, &renderGraph, &shadowAtlasRef, &cameraFrustumCorners, scene, pushRenderingStatsCallback, nearDistance, farDistance](
			const entt::entity entityHandle,
			const Grindstone::TagComponent& tag,
			DirectionalLightComponent& directionalLightComponent
			) {
				const ECS::Entity entity = ECS::Entity(entityHandle, scene);
				uint32_t cascadeCount = glm::clamp(directionalLightComponent.cascadeCount, 0u, 8u);
				Grindstone::Math::IntRect2D renderArea;

				float cascadeDistances[9]{};
				for (size_t cascadeIndex = 0; cascadeIndex <= cascadeCount + 1; ++cascadeIndex) {
					cascadeDistances[cascadeIndex] = GetCascadePlane(cascadeIndex, cascadeCount);
				}

				for (size_t cascadeIndex = 0; cascadeIndex < cascadeCount; ++cascadeIndex) {
					if (GetAtlasRenderArea(renderArea)) {
						Grindstone::Renderer::MetaRect metaRect = MetaRect::Pixels(
							static_cast<uint32_t>(renderArea.offset.x),
							static_cast<uint32_t>(renderArea.offset.y),
							renderArea.extent.x,
							renderArea.extent.y
						);

						const Math::Float3 lightDirection = entity.GetWorldForward();
						const auto [lightProjectionMatrix, lightViewMatrix] = GenerateCascadeProjectionViewMatrix(
							cameraFrustumCorners,
							lightDirection,
							cascadeDistances[cascadeIndex + 0],
							cascadeDistances[cascadeIndex + 1]
						);

						float cascadeDistance = glm::mix(nearDistance, farDistance, cascadeDistances[cascadeIndex + 1]);
						directionalLightComponent.cascadeDistances[cascadeIndex] = cascadeDistance;

						shadowAtlasRef = AddDirectionalShadowPass(renderGraph, tag.tag, shadowAtlasRef, metaRect, entity, directionalLightComponent, pushRenderingStatsCallback, lightProjectionMatrix, lightViewMatrix, cascadeIndex);
					}
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
