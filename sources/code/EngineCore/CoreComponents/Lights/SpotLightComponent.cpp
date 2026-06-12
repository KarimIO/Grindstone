#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <Common/Graphics/DescriptorSet.hpp>
#include <Common/Graphics/DescriptorSetLayout.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include "SpotLightComponent.hpp"

using namespace Grindstone;

REFLECT_STRUCT_BEGIN(SpotLightComponent)
	REFLECT_STRUCT_MEMBER(color)
	REFLECT_STRUCT_MEMBER(attenuationRadius)
	REFLECT_STRUCT_MEMBER(intensity)
	REFLECT_STRUCT_MEMBER(innerAngle)
	REFLECT_STRUCT_MEMBER(outerAngle)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::SetupSpotLightComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	auto& engineCore = EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	SpotLightComponent& spotLightComponent = cxtSet.GetEntityRegistry().get<SpotLightComponent>(entity);

	{
		SpotLightComponent::UniformStruct lightStruct{};
		GraphicsAPI::Buffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.content = &lightStruct;
		lightUniformBufferObjectCi.debugName = "LightUbo";
		lightUniformBufferObjectCi.bufferUsage = GraphicsAPI::BufferUsage::Uniform;
		lightUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
		lightUniformBufferObjectCi.bufferSize = sizeof(SpotLightComponent::UniformStruct);
		spotLightComponent.uniformBufferObject = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);

		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 1> lightLayoutBindings{};
		lightLayoutBindings[0].bindingId = 0;
		lightLayoutBindings[0].count = 1;
		lightLayoutBindings[0].type = GraphicsAPI::BindingType::UniformBuffer;
		lightLayoutBindings[0].stages = GraphicsAPI::ShaderStageBit::Fragment;

		GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Spotlight Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(lightLayoutBindings.size());
		descriptorSetLayoutCreateInfo.bindings = lightLayoutBindings.data();
		spotLightComponent.descriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(descriptorSetLayoutCreateInfo);

		std::array<GraphicsAPI::DescriptorSet::Binding, 1> lightBindings{
			GraphicsAPI::DescriptorSet::Binding::UniformBuffer( spotLightComponent.uniformBufferObject )
		};

		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Spotlight Descriptor Set";
		descriptorSetCreateInfo.layout = spotLightComponent.descriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(lightBindings.size());
		descriptorSetCreateInfo.bindings = lightBindings.data();
		spotLightComponent.descriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	{
		GraphicsAPI::Buffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "Spot Shadow Map";
		lightUniformBufferObjectCi.bufferUsage = GraphicsAPI::BufferUsage::Uniform;
		lightUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
		lightUniformBufferObjectCi.bufferSize = sizeof(glm::mat4);
		spotLightComponent.shadowMapUniformBufferObject = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);

		GraphicsAPI::DescriptorSetLayout::Binding lightUboBindingLayout{};
		lightUboBindingLayout.bindingId = 0;
		lightUboBindingLayout.count = 1;
		lightUboBindingLayout.type = GraphicsAPI::BindingType::UniformBuffer;
		lightUboBindingLayout.stages = GraphicsAPI::ShaderStageBit::Vertex;

		GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Spotlight Shadow Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = 1;
		descriptorSetLayoutCreateInfo.bindings = &lightUboBindingLayout;
		spotLightComponent.shadowMapDescriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(descriptorSetLayoutCreateInfo);

		GraphicsAPI::DescriptorSet::Binding lightUboBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( spotLightComponent.shadowMapUniformBufferObject );

		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Spotlight Shadow Descriptor Set";
		descriptorSetCreateInfo.layout = spotLightComponent.shadowMapDescriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = 1;
		descriptorSetCreateInfo.bindings = &lightUboBinding;
		spotLightComponent.shadowMapDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}

void Grindstone::DestroySpotLightComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	SpotLightComponent& spotLightComponent = cxtSet.GetEntityRegistry().get<SpotLightComponent>(entity);

	EngineCore::GetInstance().PushDeletion(
		[spotLightComponent]() {
			EngineCore& engineCore = EngineCore::GetInstance();
			GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

			graphicsCore->DeleteDescriptorSet(spotLightComponent.shadowMapDescriptorSet);
			graphicsCore->DeleteDescriptorSetLayout(spotLightComponent.shadowMapDescriptorSetLayout);
			graphicsCore->DeleteDescriptorSet(spotLightComponent.descriptorSet);
			graphicsCore->DeleteDescriptorSetLayout(spotLightComponent.descriptorSetLayout);
			graphicsCore->DeleteBuffer(spotLightComponent.shadowMapUniformBufferObject);
			graphicsCore->DeleteBuffer(spotLightComponent.uniformBufferObject);
		}
	);

	spotLightComponent.shadowMapDescriptorSet = nullptr;
	spotLightComponent.shadowMapDescriptorSetLayout = nullptr;
	spotLightComponent.descriptorSet = nullptr;
	spotLightComponent.descriptorSetLayout = nullptr;
	spotLightComponent.shadowMapUniformBufferObject = nullptr;
	spotLightComponent.uniformBufferObject = nullptr;
}
