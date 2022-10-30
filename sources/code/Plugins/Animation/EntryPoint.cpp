#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "Assets/AnimationClipAsset.hpp"
#include "Assets/AnimationClipImporter.hpp"
#include "Components/AnimatorComponent.hpp"
using namespace Grindstone;

extern "C" {
	RENDERABLES_3D_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		pluginInterface->RegisterComponent<Grindstone::AnimatorComponent>();
		pluginInterface->RegisterAssetType<AnimationClipAsset, AnimationClipImporter>();
	}

	RENDERABLES_3D_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
