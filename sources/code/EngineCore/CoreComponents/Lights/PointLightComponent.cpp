#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <Common/Graphics/DescriptorSet.hpp>
#include <Common/Graphics/DescriptorSetLayout.hpp>
#include "PointLightComponent.hpp"

using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

REFLECT_STRUCT_BEGIN(PointLightComponent)
	REFLECT_STRUCT_MEMBER(color)
	REFLECT_STRUCT_MEMBER(attenuationRadius)
	REFLECT_STRUCT_MEMBER(intensity)
	// REFLECT_STRUCT_MEMBER(shadowResolution)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::PointLightComponent::Construct(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	auto& engineCore = EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	PointLightComponent& pointLightComponent = cxtSet.GetEntityRegistry().get<PointLightComponent>(entity);

	{
		PointLightComponent::UniformStruct lightStruct{};
		GraphicsAPI::Buffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.content = &lightStruct;
		lightUniformBufferObjectCi.debugName = "LightUbo";
		lightUniformBufferObjectCi.bufferUsage = GraphicsAPI::BufferUsage::Uniform;
		lightUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
		lightUniformBufferObjectCi.bufferSize = sizeof(PointLightComponent::UniformStruct);
		pointLightComponent.uniformBufferObject = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);

		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 1> lightLayoutBindings{};
		lightLayoutBindings[0].bindingId = 0;
		lightLayoutBindings[0].count = 1;
		lightLayoutBindings[0].type = GraphicsAPI::BindingType::UniformBuffer;
		lightLayoutBindings[0].stages = GraphicsAPI::ShaderStageBit::Fragment;

		GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Pointlight Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(lightLayoutBindings.size());
		descriptorSetLayoutCreateInfo.bindings = lightLayoutBindings.data();
		pointLightComponent.descriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(descriptorSetLayoutCreateInfo);

		std::array<GraphicsAPI::DescriptorSet::Binding, 1> lightBindings{
			GraphicsAPI::DescriptorSet::Binding::UniformBuffer(pointLightComponent.uniformBufferObject)
		};

		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Pointlight Descriptor Set";
		descriptorSetCreateInfo.layout = pointLightComponent.descriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(lightBindings.size());
		descriptorSetCreateInfo.bindings = lightBindings.data();
		pointLightComponent.descriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	{
		GraphicsAPI::DescriptorSetLayout::Binding lightUboBindingLayout{};
		lightUboBindingLayout.bindingId = 0;
		lightUboBindingLayout.count = 1;
		lightUboBindingLayout.type = GraphicsAPI::BindingType::UniformBuffer;
		lightUboBindingLayout.stages = GraphicsAPI::ShaderStageBit::Vertex;

		GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Pointlight Shadow Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = 1;
		descriptorSetLayoutCreateInfo.bindings = &lightUboBindingLayout;
		pointLightComponent.shadowMapDescriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(descriptorSetLayoutCreateInfo);

		for (size_t i = 0; i < 6; ++i) {
			GraphicsAPI::Buffer::CreateInfo lightUniformBufferObjectCi{};
			lightUniformBufferObjectCi.debugName = "Point Shadow Map";
			lightUniformBufferObjectCi.bufferUsage = GraphicsAPI::BufferUsage::Uniform;
			lightUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
			lightUniformBufferObjectCi.bufferSize = sizeof(glm::mat4);
			pointLightComponent.shadowMapUniformBufferObjects[i] = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);

			GraphicsAPI::DescriptorSet::Binding lightUboBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(pointLightComponent.shadowMapUniformBufferObjects[i]);

			GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
			descriptorSetCreateInfo.debugName = "Pointlight Shadow Descriptor Set";
			descriptorSetCreateInfo.layout = pointLightComponent.shadowMapDescriptorSetLayout;
			descriptorSetCreateInfo.bindingCount = 1;
			descriptorSetCreateInfo.bindings = &lightUboBinding;
			pointLightComponent.shadowMapDescriptorSets[i] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
		}
	}
}

void Grindstone::PointLightComponent::Destroy(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	EngineCore& engineCore = EngineCore::GetInstance();
	PointLightComponent& pointLightComponent = cxtSet.GetEntityRegistry().get<PointLightComponent>(entity);

	engineCore.PushDeletion(
		[pointLightComponent]() {
			EngineCore& engineCore = EngineCore::GetInstance();
			GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

			graphicsCore->DeleteDescriptorSet(pointLightComponent.descriptorSet);
			graphicsCore->DeleteDescriptorSetLayout(pointLightComponent.descriptorSetLayout);
			graphicsCore->DeleteBuffer(pointLightComponent.uniformBufferObject);
		}
	);

	pointLightComponent.descriptorSet = nullptr;
	pointLightComponent.descriptorSetLayout = nullptr;
	pointLightComponent.uniformBufferObject = nullptr;
}
