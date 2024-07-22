#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "Assets/Mesh3dAsset.hpp"
#include "Assets/Mesh3dImporter.hpp"
#include "Assets/RigAsset.hpp"
#include "Assets/RigImporter.hpp"
#include "Mesh3dRenderer.hpp"
#include "EngineCore/Assets/Shaders/ShaderAsset.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/MeshRendererComponent.hpp"
#include <Common/Graphics/Core.hpp>
using namespace Grindstone;

extern "C" {
	RENDERABLES_3D_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());

		EngineCore* engineCore = pluginInterface->GetEngineCore();

		pluginInterface->RegisterComponent<MeshComponent>();
		pluginInterface->RegisterComponent<MeshRendererComponent>();
		pluginInterface->RegisterAssetType(Mesh3dAsset::GetStaticType(), "Mesh3dAsset", new Mesh3dImporter(engineCore));
		pluginInterface->RegisterAssetRenderer(new Mesh3dRenderer(engineCore));
	}

	RENDERABLES_3D_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
