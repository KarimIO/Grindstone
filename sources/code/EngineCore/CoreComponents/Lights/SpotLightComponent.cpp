#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/UniformBuffer.hpp>
#include <Common/Graphics/DescriptorSet.hpp>
#include <Common/Graphics/DescriptorSetLayout.hpp>
#include "SpotLightComponent.hpp"

using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

REFLECT_STRUCT_BEGIN(SpotLightComponent)
	REFLECT_STRUCT_MEMBER(color)
	REFLECT_STRUCT_MEMBER(attenuationRadius)
	REFLECT_STRUCT_MEMBER(intensity)
	REFLECT_STRUCT_MEMBER(innerAngle)
	REFLECT_STRUCT_MEMBER(outerAngle)
	REFLECT_STRUCT_MEMBER(shadowResolution)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::SetupSpotLightComponent(ECS::Entity& entity, void* componentPtr) {
	auto& engineCore = EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	SpotLightComponent& spotLightComponent = *static_cast<SpotLightComponent*>(componentPtr);

	UniformBuffer::CreateInfo lightUniformBufferObjectCi{};
	lightUniformBufferObjectCi.debugName = "LightUbo";
	lightUniformBufferObjectCi.isDynamic = true;
	lightUniformBufferObjectCi.size = sizeof(SpotLightComponent::UniformStruct);
	spotLightComponent.uniformBufferObject = graphicsCore->CreateUniformBuffer(lightUniformBufferObjectCi);

	SpotLightComponent::UniformStruct lightStruct{};
	spotLightComponent.uniformBufferObject->UpdateBuffer(&lightStruct);

	DescriptorSetLayout::Binding lightUboBindingLayout{};
	lightUboBindingLayout.bindingId = 0;
	lightUboBindingLayout.count = 1;
	lightUboBindingLayout.type = BindingType::UniformBuffer;
	lightUboBindingLayout.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::CreateInfo engineDescriptorSetLayoutCreateInfo{};
	engineDescriptorSetLayoutCreateInfo.debugName = "Light UBO Descriptor Set Layout";
	engineDescriptorSetLayoutCreateInfo.bindingCount = 1;
	engineDescriptorSetLayoutCreateInfo.bindings = &lightUboBindingLayout;
	spotLightComponent.descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(engineDescriptorSetLayoutCreateInfo);

	DescriptorSet::Binding lightUboBinding{};
	lightUboBinding.bindingIndex = 0;
	lightUboBinding.count = 1;
	lightUboBinding.bindingType = BindingType::UniformBuffer;
	lightUboBinding.itemPtr = spotLightComponent.uniformBufferObject;

	DescriptorSet::CreateInfo engineDescriptorSetCreateInfo{};
	engineDescriptorSetCreateInfo.debugName = "Light UBO Descriptor Set";
	engineDescriptorSetCreateInfo.layout = spotLightComponent.descriptorSetLayout;
	engineDescriptorSetCreateInfo.bindingCount = 1;
	engineDescriptorSetCreateInfo.bindings = &lightUboBinding;
	spotLightComponent.descriptorSet = graphicsCore->CreateDescriptorSet(engineDescriptorSetCreateInfo);
}
