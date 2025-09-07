#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/CoreComponents/Tag/TagComponent.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

#include <Grindstone.Renderables.3D//include/pch.hpp>
#include <Grindstone.Renderables.3D//include/Assets/Mesh3dAsset.hpp>
#include <Grindstone.Renderables.3D//include/Assets/Mesh3dImporter.hpp>
#include <Grindstone.Renderables.3D//include/Assets/RigAsset.hpp>
#include <Grindstone.Renderables.3D//include/Assets/RigImporter.hpp>
#include <Grindstone.Renderables.3D//include/Mesh3dRenderer.hpp>
#include <Grindstone.Renderables.3D//include/Components/MeshComponent.hpp>
#include <Grindstone.Renderables.3D//include/Components/MeshRendererComponent.hpp>

using namespace Grindstone;
using namespace Grindstone::Memory;

Mesh3dImporter* mesh3dImporter = nullptr;
Mesh3dRenderer* mesh3dRenderer = nullptr;

static void SetupMeshRendererComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	auto [tagComponent, meshRendererComponent] = cxtSet.GetEntityRegistry().get<TagComponent, MeshRendererComponent>(entity);

	{
		std::string debugName = tagComponent.tag + " Per Draw Uniform Buffer";
		GraphicsAPI::Buffer::CreateInfo uniformBufferCreateInfo{};
		uniformBufferCreateInfo.debugName = debugName.c_str();
		uniformBufferCreateInfo.bufferUsage =
			GraphicsAPI::BufferUsage::TransferDst |
			GraphicsAPI::BufferUsage::TransferSrc |
			GraphicsAPI::BufferUsage::Uniform;
		uniformBufferCreateInfo.memoryUsage = GraphicsAPI::MemUsage::CPUToGPU;
		uniformBufferCreateInfo.bufferSize = sizeof(float) * 16 + sizeof(uint32_t);
		meshRendererComponent.perDrawUniformBuffer = graphicsCore->CreateBuffer(uniformBufferCreateInfo);
	}

	GraphicsAPI::DescriptorSet::Binding descriptorSetUniformBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( meshRendererComponent.perDrawUniformBuffer );

	{
		std::string debugName = tagComponent.tag + " Per Draw Descriptor Set";
		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = debugName.c_str();
		descriptorSetCreateInfo.bindingCount = 1;
		descriptorSetCreateInfo.bindings = &descriptorSetUniformBinding;
		descriptorSetCreateInfo.layout = mesh3dRenderer->GetPerDrawDescriptorSetLayout();
		meshRendererComponent.perDrawDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}

static void DestroyMeshRendererComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	MeshRendererComponent& meshRendererComponent = cxtSet.GetEntityRegistry().get<MeshRendererComponent>(entity);
	graphicsCore->DeleteDescriptorSet(meshRendererComponent.perDrawDescriptorSet);
	graphicsCore->DeleteBuffer(meshRendererComponent.perDrawUniformBuffer);
}


extern "C" {
	RENDERABLES_3D_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		Grindstone::EngineCore* engineCore = pluginInterface->GetEngineCore();
		EngineCore::SetInstance(*engineCore);

		mesh3dImporter = AllocatorCore::Allocate<Mesh3dImporter>(engineCore);
		mesh3dRenderer = AllocatorCore::Allocate<Mesh3dRenderer>(engineCore);

		pluginInterface->RegisterAssetType(Mesh3dAsset::GetStaticType(), "Mesh3dAsset", mesh3dImporter);
		pluginInterface->RegisterComponent<MeshComponent>();
		pluginInterface->RegisterComponent<MeshRendererComponent>(SetupMeshRendererComponent, DestroyMeshRendererComponent);
		pluginInterface->RegisterAssetRenderer(mesh3dRenderer);
	}

	RENDERABLES_3D_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		if (mesh3dRenderer) {
			pluginInterface->UnregisterAssetRenderer(mesh3dRenderer);
			AllocatorCore::Free(mesh3dRenderer);
			mesh3dRenderer = nullptr;
		}

		pluginInterface->UnregisterComponent<MeshRendererComponent>();
		pluginInterface->UnregisterComponent<MeshComponent>();

		if (mesh3dImporter) {
			pluginInterface->UnregisterAssetType(Mesh3dAsset::GetStaticType());
			AllocatorCore::Free(mesh3dImporter);
			mesh3dImporter = nullptr;
		}
	}
}
