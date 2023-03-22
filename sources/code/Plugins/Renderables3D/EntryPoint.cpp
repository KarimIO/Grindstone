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
using namespace Grindstone;

Assets::AssetManager* assetManager = nullptr;
Mesh3dRenderer* assetRenderer = nullptr;
Mesh3dImporter* meshImporter = nullptr;

void SetupMeshRendererComponent(ECS::Entity& entity, void* componentPtr) {
	MeshRendererComponent* meshRenderer = (MeshRendererComponent*)componentPtr;

	if (!entity.HasComponent<MeshComponent>()) {
		return;
	}

	MeshComponent& meshComponent = entity.GetComponent<MeshComponent>();
	Mesh3dAsset* meshAsset = static_cast<Mesh3dAsset*>(meshComponent.mesh.asset);

	if (meshAsset == nullptr) {
		return;
	}

	auto& submeshes = meshAsset->submeshes;

	if (meshRenderer->materials.size() == 0) {
		return;
	}

	std::vector<MaterialAsset*> materialAssets(meshRenderer->materials.size());
	for (size_t i = 0; i < meshRenderer->materials.size(); ++i) {
		Uuid materialUuid = meshRenderer->materials[i].uuid;
		MaterialAsset* materialAsset = assetManager->GetAsset<MaterialAsset>(materialUuid);
		materialAssets[i] = materialAsset;
		const char* renderQueue = "Opaque";
		if (materialAsset != nullptr) {
			assetRenderer->AddShaderToRenderQueue(materialAsset->shaderUuid, renderQueue);
		}
	}

	for (size_t i = 0; i < submeshes.size(); ++i) {
		int materialIndex = submeshes[i].materialIndex;

		if (materialIndex >= meshRenderer->materials.size()) {
			materialIndex = 0;
		}

		MaterialAsset* materialAsset = materialAssets[materialIndex];
		if (materialAsset != nullptr) {
			materialAsset->renderables.emplace_back(entity, &submeshes[i]);
		}
	}
}

extern "C" {
	RENDERABLES_3D_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		EngineCore* engineCore = pluginInterface->GetEngineCore();
		assetManager = engineCore->assetManager;
		assetRenderer = new Mesh3dRenderer(pluginInterface->GetEngineCore());
		meshImporter = new Mesh3dImporter(engineCore);

		pluginInterface->RegisterComponent<MeshComponent>();
		pluginInterface->RegisterComponent<MeshRendererComponent>(SetupMeshRendererComponent);
		pluginInterface->RegisterAssetType(Mesh3dAsset::GetStaticType(), "Mesh3dAsset", meshImporter);
		pluginInterface->RegisterAssetRenderer(assetRenderer);
	}

	RENDERABLES_3D_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
