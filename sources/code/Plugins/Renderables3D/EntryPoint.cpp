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

GraphicsAPI::DescriptorSetLayout* perDrawDescriptorSetLayout = nullptr;
GraphicsAPI::Core* graphicsCore = nullptr;
Assets::AssetManager* assetManager = nullptr;
Mesh3dRenderer* assetRenderer = nullptr;
Mesh3dImporter* meshImporter = nullptr;

void SetupMeshRendererComponent(ECS::Entity& entity, void* componentPtr) {
	MeshRendererComponent* meshRenderer = (MeshRendererComponent*)componentPtr;

	if (!entity.HasComponent<MeshComponent>()) {
		return;
	}

	std::vector<MaterialAsset*> materialAssets(meshRenderer->materials.size());
	for (size_t i = 0; i < meshRenderer->materials.size(); ++i) {
		Uuid materialUuid = meshRenderer->materials[i].uuid;
		MaterialAsset* materialAsset = assetManager->GetAsset<MaterialAsset>(materialUuid);
		materialAssets[i] = materialAsset;
	}

	GraphicsAPI::UniformBuffer::CreateInfo uniformBufferCreateInfo{};
	uniformBufferCreateInfo.debugName = "Per Draw Uniform Buffer";
	uniformBufferCreateInfo.isDynamic = true;
	uniformBufferCreateInfo.size = sizeof(float) * 16;
	meshRenderer->perDrawUniformBuffer = graphicsCore->CreateUniformBuffer(uniformBufferCreateInfo);

	GraphicsAPI::DescriptorSet::Binding descriptorSetUniformBinding{};
	descriptorSetUniformBinding.bindingIndex = 0;
	descriptorSetUniformBinding.bindingType = GraphicsAPI::BindingType::UniformBuffer;
	descriptorSetUniformBinding.count = 1;
	descriptorSetUniformBinding.itemPtr = meshRenderer->perDrawUniformBuffer;

	GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
	descriptorSetCreateInfo.debugName = "Per Draw Descriptor Set";
	descriptorSetCreateInfo.bindingCount = 1;
	descriptorSetCreateInfo.bindings = &descriptorSetUniformBinding;
	descriptorSetCreateInfo.layout = perDrawDescriptorSetLayout;
	meshRenderer->perDrawDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
}

extern "C" {
	RENDERABLES_3D_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		EngineCore* engineCore = pluginInterface->GetEngineCore();
		assetManager = engineCore->assetManager;
		graphicsCore = engineCore->GetGraphicsCore();
		assetRenderer = new Mesh3dRenderer(pluginInterface->GetEngineCore());
		meshImporter = new Mesh3dImporter(engineCore);

		GraphicsAPI::DescriptorSetLayout::Binding descriptorSetUniformBinding{};
		descriptorSetUniformBinding.bindingId = 0;
		descriptorSetUniformBinding.type = GraphicsAPI::BindingType::UniformBuffer;
		descriptorSetUniformBinding.count = 1;
		descriptorSetUniformBinding.stages = GraphicsAPI::ShaderStageBit::Vertex | GraphicsAPI::ShaderStageBit::Fragment;

		GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Per Draw Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = 1;
		descriptorSetLayoutCreateInfo.bindings = &descriptorSetUniformBinding;
		perDrawDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		pluginInterface->RegisterComponent<MeshComponent>();
		pluginInterface->RegisterComponent<MeshRendererComponent>(SetupMeshRendererComponent);
		pluginInterface->RegisterAssetType(Mesh3dAsset::GetStaticType(), "Mesh3dAsset", meshImporter);
		pluginInterface->RegisterAssetRenderer(assetRenderer);
	}

	RENDERABLES_3D_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
