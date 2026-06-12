#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <Common/Graphics/DescriptorSet.hpp>
#include <Common/Graphics/DescriptorSetLayout.hpp>
#include "DirectionalLightComponent.hpp"

using namespace Grindstone;

REFLECT_STRUCT_BEGIN(DirectionalLightComponent)
	REFLECT_STRUCT_MEMBER(color)
	REFLECT_STRUCT_MEMBER(sourceRadius)
	REFLECT_STRUCT_MEMBER(intensity)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::SetupDirectionalLightComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	auto& engineCore = EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	DirectionalLightComponent& directionalLightComponent = cxtSet.GetEntityRegistry().get<DirectionalLightComponent>(entity);

	{
		DirectionalLightComponent::UniformStruct lightStruct{};
		GraphicsAPI::Buffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "LightUbo";
		lightUniformBufferObjectCi.content = &lightStruct;
		lightUniformBufferObjectCi.bufferUsage = GraphicsAPI::BufferUsage::Uniform;
		lightUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
		lightUniformBufferObjectCi.bufferSize = sizeof(DirectionalLightComponent::UniformStruct);
		directionalLightComponent.uniformBufferObject = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);

		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 1> lightLayoutBindings{};
		lightLayoutBindings[0].bindingId = 0;
		lightLayoutBindings[0].count = 1;
		lightLayoutBindings[0].type = GraphicsAPI::BindingType::UniformBuffer;
		lightLayoutBindings[0].stages = GraphicsAPI::ShaderStageBit::Fragment;

		GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Directional Light Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(lightLayoutBindings.size());
		descriptorSetLayoutCreateInfo.bindings = lightLayoutBindings.data();
		directionalLightComponent.descriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(descriptorSetLayoutCreateInfo);

		std::array<GraphicsAPI::DescriptorSet::Binding, 1> lightBindings{
			GraphicsAPI::DescriptorSet::Binding::UniformBuffer( directionalLightComponent.uniformBufferObject )
		};

		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Directional Light Descriptor Set";
		descriptorSetCreateInfo.layout = directionalLightComponent.descriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(lightBindings.size());
		descriptorSetCreateInfo.bindings = lightBindings.data();
		directionalLightComponent.descriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	{
		GraphicsAPI::Buffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "Directional Light Shadow Map";
		lightUniformBufferObjectCi.bufferUsage = GraphicsAPI::BufferUsage::Uniform;
		lightUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
		lightUniformBufferObjectCi.bufferSize = sizeof(glm::mat4);
		directionalLightComponent.shadowMapUniformBufferObject = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);

		GraphicsAPI::DescriptorSetLayout::Binding lightUboBindingLayout{};
		lightUboBindingLayout.bindingId = 0;
		lightUboBindingLayout.count = 1;
		lightUboBindingLayout.type = GraphicsAPI::BindingType::UniformBuffer;
		lightUboBindingLayout.stages = GraphicsAPI::ShaderStageBit::Vertex;

		GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Directional Light Shadow Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = 1;
		descriptorSetLayoutCreateInfo.bindings = &lightUboBindingLayout;
		directionalLightComponent.shadowMapDescriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(descriptorSetLayoutCreateInfo);

		GraphicsAPI::DescriptorSet::Binding lightUboBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( directionalLightComponent.shadowMapUniformBufferObject );

		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Directional Light Shadow Descriptor Set";
		descriptorSetCreateInfo.layout = directionalLightComponent.shadowMapDescriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = 1;
		descriptorSetCreateInfo.bindings = &lightUboBinding;
		directionalLightComponent.shadowMapDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}

void Grindstone::DestroyDirectionalLightComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	DirectionalLightComponent& directionalLightComponent = cxtSet.GetEntityRegistry().get<DirectionalLightComponent>(entity);

	EngineCore::GetInstance().PushDeletion(
		[directionalLightComponent]() {
			EngineCore& engineCore = EngineCore::GetInstance();
			GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

			graphicsCore->DeleteDescriptorSet(directionalLightComponent.shadowMapDescriptorSet);
			graphicsCore->DeleteDescriptorSetLayout(directionalLightComponent.shadowMapDescriptorSetLayout);
			graphicsCore->DeleteDescriptorSet(directionalLightComponent.descriptorSet);
			graphicsCore->DeleteDescriptorSetLayout(directionalLightComponent.descriptorSetLayout);
			graphicsCore->DeleteBuffer(directionalLightComponent.shadowMapUniformBufferObject);
			graphicsCore->DeleteBuffer(directionalLightComponent.uniformBufferObject);
		}
	);

	directionalLightComponent.shadowMapDescriptorSet = nullptr;
	directionalLightComponent.shadowMapDescriptorSetLayout = nullptr;
	directionalLightComponent.descriptorSet = nullptr;
	directionalLightComponent.descriptorSetLayout = nullptr;
	directionalLightComponent.shadowMapUniformBufferObject = nullptr;
	directionalLightComponent.uniformBufferObject = nullptr;
}
