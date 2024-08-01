#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <Common/Graphics/Core.hpp>

#include "Assets/Mesh3dAsset.hpp"
#include "Assets/Mesh3dImporter.hpp"
#include "Assets/RigAsset.hpp"
#include "Assets/RigImporter.hpp"
#include "Mesh3dRenderer.hpp"
#include "EngineCore/Assets/Shaders/ShaderAsset.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/MeshRendererComponent.hpp"
using namespace Grindstone;
using namespace Grindstone::Memory;

Mesh3dImporter* mesh3dImporter = nullptr;
Mesh3dRenderer* mesh3dRenderer = nullptr;

extern "C" {
	RENDERABLES_3D_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		EngineCore* engineCore = pluginInterface->GetEngineCore();

		mesh3dImporter = AllocatorCore::Allocate<Mesh3dImporter>(engineCore);
		mesh3dRenderer = AllocatorCore::Allocate<Mesh3dRenderer>(engineCore);

		pluginInterface->RegisterComponent<MeshComponent>();
		pluginInterface->RegisterComponent<MeshRendererComponent>();
		pluginInterface->RegisterAssetType(Mesh3dAsset::GetStaticType(), "Mesh3dAsset", mesh3dImporter);
		pluginInterface->RegisterAssetRenderer(mesh3dRenderer);
	}

	RENDERABLES_3D_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		if (mesh3dRenderer) {
			pluginInterface->UnregisterAssetRenderer(mesh3dRenderer);
			AllocatorCore::Free(mesh3dRenderer);
			mesh3dRenderer = nullptr;
		}

		if (mesh3dImporter) {
			pluginInterface->UnregisterAssetType(Mesh3dAsset::GetStaticType());
			AllocatorCore::Free(mesh3dImporter);
			mesh3dImporter = nullptr;
		}

		pluginInterface->UnregisterComponent<MeshRendererComponent>();
		pluginInterface->UnregisterComponent<MeshComponent>();
	}
}
