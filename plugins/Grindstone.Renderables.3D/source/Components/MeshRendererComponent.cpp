#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/CoreComponents/Tag/TagComponent.hpp>

#include <Grindstone.Renderables.3D/include/Components/MeshRendererComponent.hpp>
#include <Grindstone.Renderables.3D/include/Mesh3dRenderer.hpp>

using namespace Grindstone;

REFLECT_STRUCT_BEGIN(MeshRendererComponent)
	REFLECT_STRUCT_MEMBER(materials)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::MeshRendererComponent::Construct(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
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
		uniformBufferCreateInfo.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
		uniformBufferCreateInfo.bufferSize = sizeof(float) * 16 + sizeof(uint32_t);
		meshRendererComponent.perDrawUniformBuffer = graphicsCore->CreateBuffer(uniformBufferCreateInfo);
	}

	GraphicsAPI::DescriptorSet::Binding descriptorSetUniformBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(meshRendererComponent.perDrawUniformBuffer);

	{
		std::string debugName = tagComponent.tag + " Per Draw Descriptor Set";
		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = debugName.c_str();
		descriptorSetCreateInfo.bindingCount = 1;
		descriptorSetCreateInfo.bindings = &descriptorSetUniformBinding;
		descriptorSetCreateInfo.layout = Mesh3dRenderer::GetPerDrawDescriptorSetLayout();
		meshRendererComponent.perDrawDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}

void Grindstone::MeshRendererComponent::Destroy(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	MeshRendererComponent& meshRendererComponent = cxtSet.GetEntityRegistry().get<MeshRendererComponent>(entity);

	EngineCore::GetInstance().PushDeletion(
		[meshRendererComponent]() {
			GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
			graphicsCore->DeleteDescriptorSet(meshRendererComponent.perDrawDescriptorSet);
			graphicsCore->DeleteBuffer(meshRendererComponent.perDrawUniformBuffer);
		}
	);

	meshRendererComponent.perDrawDescriptorSet = nullptr;
	meshRendererComponent.perDrawUniformBuffer = nullptr;
}
