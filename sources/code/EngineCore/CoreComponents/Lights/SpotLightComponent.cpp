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

	uint32_t shadowResolution = static_cast<uint32_t>(spotLightComponent.shadowResolution);

	RenderPass::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.width = shadowResolution;
	renderPassCreateInfo.height = shadowResolution;
	renderPassCreateInfo.colorFormats = nullptr;
	renderPassCreateInfo.colorFormatCount = 0;
	renderPassCreateInfo.depthFormat = DepthFormat::D32;
	spotLightComponent.renderPass = graphicsCore->CreateRenderPass(renderPassCreateInfo);

	DepthTarget::CreateInfo gbufferDepthImageCreateInfo(renderPassCreateInfo.depthFormat, shadowResolution, shadowResolution, false, false, true, "Shadow Map Depth Image");
	spotLightComponent.depthTarget = graphicsCore->CreateDepthTarget(gbufferDepthImageCreateInfo);

	Framebuffer::CreateInfo gbufferCreateInfo{};
	gbufferCreateInfo.debugName = "G-Buffer Framebuffer";
	gbufferCreateInfo.renderPass = spotLightComponent.renderPass;
	gbufferCreateInfo.renderTargetLists = nullptr;
	gbufferCreateInfo.numRenderTargetLists = 0;
	gbufferCreateInfo.depthTarget = spotLightComponent.depthTarget;
	spotLightComponent.framebuffer = graphicsCore->CreateFramebuffer(gbufferCreateInfo);

	{
		UniformBuffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "LightUbo";
		lightUniformBufferObjectCi.isDynamic = true;
		lightUniformBufferObjectCi.size = sizeof(SpotLightComponent::UniformStruct);
		spotLightComponent.uniformBufferObject = graphicsCore->CreateUniformBuffer(lightUniformBufferObjectCi);

		SpotLightComponent::UniformStruct lightStruct{};
		spotLightComponent.uniformBufferObject->UpdateBuffer(&lightStruct);

		std::array<DescriptorSetLayout::Binding, 2> lightLayoutBindings{};
		lightLayoutBindings[0].bindingId = 0;
		lightLayoutBindings[0].count = 1;
		lightLayoutBindings[0].type = BindingType::UniformBuffer;
		lightLayoutBindings[0].stages = ShaderStageBit::Fragment;

		lightLayoutBindings[1].bindingId = 1;
		lightLayoutBindings[1].count = 1;
		lightLayoutBindings[1].type = BindingType::DepthTexture;
		lightLayoutBindings[1].stages = ShaderStageBit::Fragment;

		DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Light UBO Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(lightLayoutBindings.size());
		descriptorSetLayoutCreateInfo.bindings = lightLayoutBindings.data();
		spotLightComponent.descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		std::array<DescriptorSet::Binding, 2> lightBindings{};
		lightBindings[0].bindingIndex = 0;
		lightBindings[0].count = 1;
		lightBindings[0].bindingType = BindingType::UniformBuffer;
		lightBindings[0].itemPtr = spotLightComponent.uniformBufferObject;

		lightBindings[1].bindingIndex = 1;
		lightBindings[1].count = 1;
		lightBindings[1].bindingType = BindingType::DepthTexture;
		lightBindings[1].itemPtr = spotLightComponent.depthTarget;

		DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Light UBO Descriptor Set";
		descriptorSetCreateInfo.layout = spotLightComponent.descriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(lightBindings.size());
		descriptorSetCreateInfo.bindings = lightBindings.data();
		spotLightComponent.descriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	{
		UniformBuffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "Spot Shadow Map";
		lightUniformBufferObjectCi.isDynamic = true;
		lightUniformBufferObjectCi.size = sizeof(glm::mat4);
		spotLightComponent.shadowMapUniformBufferObject = graphicsCore->CreateUniformBuffer(lightUniformBufferObjectCi);

		DescriptorSetLayout::Binding lightUboBindingLayout{};
		lightUboBindingLayout.bindingId = 0;
		lightUboBindingLayout.count = 1;
		lightUboBindingLayout.type = BindingType::UniformBuffer;
		lightUboBindingLayout.stages = ShaderStageBit::Vertex;

		DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Light UBO Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = 1;
		descriptorSetLayoutCreateInfo.bindings = &lightUboBindingLayout;
		spotLightComponent.shadowMapDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		DescriptorSet::Binding lightUboBinding{};
		lightUboBinding.bindingIndex = 0;
		lightUboBinding.count = 1;
		lightUboBinding.bindingType = BindingType::UniformBuffer;
		lightUboBinding.itemPtr = spotLightComponent.shadowMapUniformBufferObject;

		DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Shadow Map Descriptor Set";
		descriptorSetCreateInfo.layout = spotLightComponent.shadowMapDescriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = 1;
		descriptorSetCreateInfo.bindings = &lightUboBinding;
		spotLightComponent.shadowMapDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}
