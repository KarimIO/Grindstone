#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/UniformBuffer.hpp>
#include <Common/Graphics/DescriptorSet.hpp>
#include <Common/Graphics/DescriptorSetLayout.hpp>
#include "PointLightComponent.hpp"

using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

REFLECT_STRUCT_BEGIN(PointLightComponent)
	REFLECT_STRUCT_MEMBER(color)
	REFLECT_STRUCT_MEMBER(attenuationRadius)
	REFLECT_STRUCT_MEMBER(intensity)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::SetupPointLightComponent(ECS::Entity& entity, void* componentPtr) {
	auto& engineCore = EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	PointLightComponent& pointLightComponent = *static_cast<PointLightComponent*>(componentPtr);

	{
		UniformBuffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "LightUbo";
		lightUniformBufferObjectCi.isDynamic = true;
		lightUniformBufferObjectCi.size = sizeof(PointLightComponent::UniformStruct);
		pointLightComponent.uniformBufferObject = graphicsCore->CreateUniformBuffer(lightUniformBufferObjectCi);

		PointLightComponent::UniformStruct lightInfoStruct{};
		pointLightComponent.uniformBufferObject->UpdateBuffer(&lightInfoStruct);
	}

	{
		DescriptorSetLayout::Binding lightUboBindingLayout{};
		lightUboBindingLayout.bindingId = 0;
		lightUboBindingLayout.count = 1;
		lightUboBindingLayout.type = BindingType::UniformBuffer;
		lightUboBindingLayout.stages = ShaderStageBit::Fragment;

		DescriptorSetLayout::CreateInfo pointLightDescriptorSetLayoutCreateInfo{};
		pointLightDescriptorSetLayoutCreateInfo.debugName = "Light UBO Descriptor Set Layout";
		pointLightDescriptorSetLayoutCreateInfo.bindingCount = 1;
		pointLightDescriptorSetLayoutCreateInfo.bindings = &lightUboBindingLayout;
		pointLightComponent.descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(pointLightDescriptorSetLayoutCreateInfo);
	}

	{
		DescriptorSet::Binding lightUboBinding{};
		lightUboBinding.bindingIndex = 0;
		lightUboBinding.count = 1;
		lightUboBinding.bindingType = BindingType::UniformBuffer;
		lightUboBinding.itemPtr = pointLightComponent.uniformBufferObject;

		DescriptorSet::CreateInfo pointLightDescriptorSetCreateInfo{};
		pointLightDescriptorSetCreateInfo.debugName = "Light UBO Descriptor Set";
		pointLightDescriptorSetCreateInfo.layout = pointLightComponent.descriptorSetLayout;
		pointLightDescriptorSetCreateInfo.bindingCount = 1;
		pointLightDescriptorSetCreateInfo.bindings = &lightUboBinding;
		pointLightComponent.descriptorSet = graphicsCore->CreateDescriptorSet(pointLightDescriptorSetCreateInfo);
	}
}
