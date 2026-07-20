#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/CoreComponents/Tag/TagComponent.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

#include <Grindstone.Renderables.3D/include/pch.hpp>
#include <Grindstone.Renderables.3D/include/AnimationSystem.hpp>
#include <Grindstone.Renderables.3D/include/Assets/AnimationClipAsset.hpp>
#include <Grindstone.Renderables.3D/include/Assets/AnimationClipImporter.hpp>
#include <Grindstone.Renderables.3D/include/Assets/Mesh3dAsset.hpp>
#include <Grindstone.Renderables.3D/include/Assets/Mesh3dImporter.hpp>
#include <Grindstone.Renderables.3D/include/Assets/RigAsset.hpp>
#include <Grindstone.Renderables.3D/include/Assets/RigImporter.hpp>
#include <Grindstone.Renderables.3D/include/Mesh3dRenderer.hpp>
#include <Grindstone.Renderables.3D/include/Components/AnimatorComponent.hpp>
#include <Grindstone.Renderables.3D/include/Components/MeshComponent.hpp>
#include <Grindstone.Renderables.3D/include/Components/MeshRendererComponent.hpp>

using namespace Grindstone;
using namespace Grindstone::Memory;

Mesh3dRenderer* mesh3dRenderer = nullptr;
Mesh3dImporter* mesh3dImporter = nullptr;
RigImporter* rigImporter = nullptr;
AnimationClipImporter* animationClipImporter = nullptr;

extern "C" {
	RENDERABLES_3D_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		Grindstone::EngineCore* engineCore = pluginInterface->GetEngineCore();
		EngineCore::SetInstance(*engineCore);

		mesh3dImporter = AllocatorCore::Allocate<Mesh3dImporter>(engineCore);
		mesh3dRenderer = AllocatorCore::Allocate<Mesh3dRenderer>(engineCore);
		animationClipImporter = AllocatorCore::Allocate<AnimationClipImporter>();
		rigImporter = AllocatorCore::Allocate<RigImporter>();

		pluginInterface->RegisterAssetType<Mesh3dAsset>(mesh3dImporter);
		pluginInterface->RegisterAssetType<RigAsset>(rigImporter);
		pluginInterface->RegisterAssetType<AnimationClipAsset>(animationClipImporter);
		pluginInterface->RegisterComponent<MeshComponent>();
		pluginInterface->RegisterComponent<MeshRendererComponent>();
		pluginInterface->RegisterComponent<AnimatorComponent>();
		pluginInterface->RegisterAssetRenderer(mesh3dRenderer);
		pluginInterface->RegisterSystem("Grindstone::AnimateSkeletonSystem", Grindstone::AnimateSkeletonSystem);
		pluginInterface->RegisterEditorSystem("Grindstone::Ed::AnimateSkeletonSystem", Grindstone::AnimateSkeletonSystem);
	}

	RENDERABLES_3D_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->UnregisterEditorSystem("Grindstone::Ed::AnimateSkeletonSystem");
		pluginInterface->UnregisterSystem("Grindstone::AnimateSkeletonSystem");

		if (mesh3dRenderer) {
			pluginInterface->UnregisterAssetRenderer(mesh3dRenderer);
			AllocatorCore::Free(mesh3dRenderer);
			mesh3dRenderer = nullptr;
		}

		pluginInterface->UnregisterComponent<AnimatorComponent>();
		pluginInterface->UnregisterComponent<MeshRendererComponent>();
		pluginInterface->UnregisterComponent<MeshComponent>();

		if (animationClipImporter) {
			pluginInterface->UnregisterAssetType<AnimationClipAsset>();
			AllocatorCore::Free(animationClipImporter);
			animationClipImporter = nullptr;
		}

		if (rigImporter) {
			pluginInterface->UnregisterAssetType<RigAsset>();
			AllocatorCore::Free(rigImporter);
			rigImporter = nullptr;
		}

		if (mesh3dImporter) {
			pluginInterface->UnregisterAssetType<Mesh3dAsset>();
			AllocatorCore::Free(mesh3dImporter);
			mesh3dImporter = nullptr;
		}
	}
}
