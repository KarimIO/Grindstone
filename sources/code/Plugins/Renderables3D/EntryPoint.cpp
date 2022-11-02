#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "Assets/Mesh3dAsset.hpp"
#include "Assets/Mesh3dImporter.hpp"
#include "Assets/RigAsset.hpp"
#include "Assets/RigImporter.hpp"
#include "Mesh3dRenderer.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/MeshRendererComponent.hpp"
using namespace Grindstone;

extern "C" {
	RENDERABLES_3D_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		pluginInterface->RegisterComponent<MeshComponent>();
		pluginInterface->RegisterComponent<MeshRendererComponent>();
		pluginInterface->RegisterAssetType<Mesh3dAsset, Mesh3dImporter>();
		pluginInterface->RegisterAssetType<RigAsset, RigImporter>();
		pluginInterface->RegisterAssetRenderer(new Mesh3dRenderer());
	}

	RENDERABLES_3D_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
