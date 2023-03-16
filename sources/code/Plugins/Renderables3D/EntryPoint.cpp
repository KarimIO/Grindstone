#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "Assets/Mesh3dAsset.hpp"
#include "Assets/Mesh3dImporter.hpp"
#include "Assets/RigAsset.hpp"
#include "Assets/RigImporter.hpp"
#include "Mesh3dRenderer.hpp"
#include "EngineCore/Assets/Shaders/ShaderAsset.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/MeshRendererComponent.hpp"
using namespace Grindstone;

Mesh3dRenderer* assetRenderer = nullptr;

void SetupMeshRendererComponent(ECS::Entity& entity, void* componentPtr) {
	MeshRendererComponent* meshRenderer = (MeshRendererComponent*)componentPtr;

	for (size_t i = 0; i < meshRenderer->materials.size(); ++i) {
		MaterialAsset* materialAsset = static_cast<MaterialAsset*>(meshRenderer->materials[i].asset);
		ShaderAsset* shaderAsset = materialAsset->shaderAsset;
		assetRenderer->AddShaderToRenderQueue(shaderAsset);
		materialAsset->renderables.emplace_back(entity, componentPtr);
	}
}

extern "C" {
	RENDERABLES_3D_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		EngineCore* engineCore = pluginInterface->GetEngineCore();
		assetRenderer = new Mesh3dRenderer(pluginInterface->GetGraphicsCore());
		Mesh3dImporter* meshImporter = new Mesh3dImporter(engineCore);

		pluginInterface->RegisterComponent<MeshComponent>();
		pluginInterface->RegisterComponent<MeshRendererComponent>(SetupMeshRendererComponent);
		pluginInterface->RegisterAssetType(Mesh3dAsset::GetStaticType(), "Mesh3dAsset", meshImporter);
		pluginInterface->RegisterAssetRenderer(assetRenderer);
	}

	RENDERABLES_3D_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
