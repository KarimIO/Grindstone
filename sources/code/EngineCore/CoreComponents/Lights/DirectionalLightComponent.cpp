#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/UniformBuffer.hpp>
#include <Common/Graphics/DescriptorSet.hpp>
#include <Common/Graphics/DescriptorSetLayout.hpp>
#include "DirectionalLightComponent.hpp"

using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

REFLECT_STRUCT_BEGIN(DirectionalLightComponent)
	REFLECT_STRUCT_MEMBER(color)
	REFLECT_STRUCT_MEMBER(sourceRadius)
	REFLECT_STRUCT_MEMBER(intensity)
	REFLECT_STRUCT_MEMBER(shadowResolution)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::SetupDirectionalLightComponent(ECS::Entity& entity, void* componentPtr) {
	auto& engineCore = EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	DirectionalLightComponent& directionalLightComponent = *static_cast<DirectionalLightComponent*>(componentPtr);

	UniformBuffer::CreateInfo lightUniformBufferObjectCi{};
	lightUniformBufferObjectCi.debugName = "LightUbo";
	lightUniformBufferObjectCi.isDynamic = true;
	lightUniformBufferObjectCi.size = sizeof(DirectionalLightComponent::UniformStruct);
	directionalLightComponent.uniformBufferObject = graphicsCore->CreateUniformBuffer(lightUniformBufferObjectCi);

	DirectionalLightComponent::UniformStruct lightStruct{};
	directionalLightComponent.uniformBufferObject->UpdateBuffer(&lightStruct);

	DescriptorSetLayout::Binding lightUboBindingLayout{};
	lightUboBindingLayout.bindingId = 0;
	lightUboBindingLayout.count = 1;
	lightUboBindingLayout.type = BindingType::UniformBuffer;
	lightUboBindingLayout.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::CreateInfo engineDescriptorSetLayoutCreateInfo{};
	engineDescriptorSetLayoutCreateInfo.debugName = "Light UBO Descriptor Set Layout";
	engineDescriptorSetLayoutCreateInfo.bindingCount = 1;
	engineDescriptorSetLayoutCreateInfo.bindings = &lightUboBindingLayout;
	directionalLightComponent.descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(engineDescriptorSetLayoutCreateInfo);

	DescriptorSet::Binding lightUboBinding{};
	lightUboBinding.bindingIndex = 0;
	lightUboBinding.count = 1;
	lightUboBinding.bindingType = BindingType::UniformBuffer;
	lightUboBinding.itemPtr = directionalLightComponent.uniformBufferObject;

	DescriptorSet::CreateInfo engineDescriptorSetCreateInfo{};
	engineDescriptorSetCreateInfo.debugName = "Light UBO Descriptor Set";
	engineDescriptorSetCreateInfo.layout = directionalLightComponent.descriptorSetLayout;
	engineDescriptorSetCreateInfo.bindingCount = 1;
	engineDescriptorSetCreateInfo.bindings = &lightUboBinding;
	directionalLightComponent.descriptorSet = graphicsCore->CreateDescriptorSet(engineDescriptorSetCreateInfo);
}
