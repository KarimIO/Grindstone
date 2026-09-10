#include <Grindstone.Editor.LightBakery/include/pch.hpp>

#include <Common/Console/Cvars.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>

#include <Grindstone.Renderer.Deferred/include/ProbeVolume.hpp>

#include <Editor/GizmoRenderer.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/PluginSystem/EditorPluginInterface.hpp>

using namespace Grindstone;
using namespace Grindstone::Memory;
using namespace Grindstone::Renderer;

namespace Grindstone::Renderer::Editor {
	static void BakeLighting() {
		GPRINT_INFO(LogSource::Rendering, "Baking Lighting...");
	}

	static const glm::vec4 probeVolumeBoxGizmoColor = glm::vec4(0.2f, 0.9f, 0.3f, 1.0f);
	static const glm::vec4 probeVolumeSphereGizmoColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	static void RenderProbeVolumeGizmos(Grindstone::Editor::GizmoRenderer& gizmoRenderer, Grindstone::Blackboard& blackboard, const Grindstone::Editor::Selection& selection) {
		Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
		Grindstone::ProbeVolume* probeVolume = nullptr;

		for (const ECS::Entity& selectedEntity : selection.selectedEntities) {
			if (selectedEntity.TryGetComponent<Grindstone::ProbeVolume>(probeVolume)) {
				Math::Box3D& boundingData = probeVolume->bounds;
				Grindstone::TransformComponent& transf = selectedEntity.GetComponent<Grindstone::TransformComponent>();
				Math::Matrix4 entityMatrix = TransformComponent::GetWorldTransformMatrix(selectedEntity);
				glm::vec3 boxSize = boundingData.extent;
				Math::Matrix4 boxMatrix = entityMatrix * glm::translate(boundingData.offset);
				gizmoRenderer.SubmitCubeGizmo(boxMatrix, boundingData.extent, probeVolumeBoxGizmoColor);

				Grindstone::Math::Uint3 probeCount(
					boundingData.extent.x / probeVolume->probeSpacing.x + 1,
					boundingData.extent.y / probeVolume->probeSpacing.y + 1,
					boundingData.extent.z / probeVolume->probeSpacing.z + 1
				);

				Grindstone::Math::Float3 probeVolumeBox(
					probeVolume->probeSpacing.x * (probeCount.x - 1),
					probeVolume->probeSpacing.y * (probeCount.y - 1),
					probeVolume->probeSpacing.z * (probeCount.z - 1)
				);
				Grindstone::Math::Float3 boxMin = boundingData.offset - probeVolumeBox / 2.0f;
				const float probeVolumeSphereGizmoRadius = glm::min(glm::min(probeVolume->probeSpacing.x, probeVolume->probeSpacing.y, probeVolume->probeSpacing.z) * 0.25f, 0.25f);

				for (uint32_t i = 0; i < probeCount.x; ++i) {
					for (uint32_t j = 0; j < probeCount.y; ++j) {
						for (uint32_t k = 0; k < probeCount.z; ++k) {
							glm::vec3 probePosition = boxMin + glm::vec3(probeVolume->probeSpacing.x * i, probeVolume->probeSpacing.y * j, probeVolume->probeSpacing.z * k);
							Math::Matrix4 sphereMatrix = entityMatrix * glm::translate(probePosition);
							gizmoRenderer.SubmitSphereGizmo(sphereMatrix, probeVolumeSphereGizmoRadius, probeVolumeSphereGizmoColor);
						}
					}
				}
			}
		}
	}
}

extern "C" {
	EDITOR_LIGHT_BAKERY_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());
		Grindstone::CvarSystem::SetInstance(pluginInterface->GetCvarSystem());

		Grindstone::EngineCore* engineCore = pluginInterface->GetEngineCore();
		EngineCore::SetInstance(*engineCore);

		Grindstone::Plugins::EditorPluginInterface* editorInterface = static_cast<Grindstone::Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());
		if (editorInterface) {
			Grindstone::Editor::Manager* editorManager = editorInterface->GetEditorInstance();
			Grindstone::Editor::GizmoRenderer& gizmoRenderer = editorManager->GetGizmoRenderer();
			gizmoRenderer.RegisterGizmoCallback("Gizmo::ProbeVolume"_hash, Grindstone::Renderer::Editor::RenderProbeVolumeGizmos);

			editorInterface->RegisterMenuItem("Bake/Bake Lighting", Grindstone::Renderer::Editor::BakeLighting);
		}
	}

	EDITOR_LIGHT_BAKERY_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Grindstone::Plugins::EditorPluginInterface* editorInterface = static_cast<Grindstone::Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());
		if (editorInterface) {
			editorInterface->DeregisterMenuItem("Bake/Bake Lighting");

			Grindstone::Editor::Manager* editorManager = editorInterface->GetEditorInstance();
			Grindstone::Editor::GizmoRenderer& gizmoRenderer = editorManager->GetGizmoRenderer();
			gizmoRenderer.UnregisterGizmoCallback("Gizmo::ProbeVolume"_hash);
		}
	}
}
