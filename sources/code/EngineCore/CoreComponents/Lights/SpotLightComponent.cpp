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
	renderPassCreateInfo.debugName = "Spotlight Shadow Render Pass";
	renderPassCreateInfo.colorFormats = nullptr;
	renderPassCreateInfo.colorFormatCount = 0;
	renderPassCreateInfo.depthFormat = DepthFormat::D32;
	spotLightComponent.renderPass = graphicsCore->CreateRenderPass(renderPassCreateInfo);

	DepthTarget::CreateInfo shadowMapDepthImageCreateInfo(renderPassCreateInfo.depthFormat, shadowResolution, shadowResolution, false, false, true, "Spot Shadow Map Depth Image");
	spotLightComponent.depthTarget = graphicsCore->CreateDepthTarget(shadowMapDepthImageCreateInfo);

	Framebuffer::CreateInfo shadowMapCreateInfo{};
	shadowMapCreateInfo.debugName = "Spotlight Shadow Framebuffer";
	shadowMapCreateInfo.width = shadowResolution;
	shadowMapCreateInfo.height = shadowResolution;
	shadowMapCreateInfo.renderPass = spotLightComponent.renderPass;
	shadowMapCreateInfo.renderTargetLists = nullptr;
	shadowMapCreateInfo.numRenderTargetLists = 0;
	shadowMapCreateInfo.depthTarget = spotLightComponent.depthTarget;
	spotLightComponent.framebuffer = graphicsCore->CreateFramebuffer(shadowMapCreateInfo);

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
		descriptorSetLayoutCreateInfo.debugName = "Spotlight Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(lightLayoutBindings.size());
		descriptorSetLayoutCreateInfo.bindings = lightLayoutBindings.data();
		spotLightComponent.descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		std::array<DescriptorSet::Binding, 2> lightBindings{
			spotLightComponent.uniformBufferObject,
			spotLightComponent.depthTarget
		};

		DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Spotlight Descriptor Set";
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
		descriptorSetLayoutCreateInfo.debugName = "Spotlight Shadow Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = 1;
		descriptorSetLayoutCreateInfo.bindings = &lightUboBindingLayout;
		spotLightComponent.shadowMapDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		DescriptorSet::Binding lightUboBinding{ spotLightComponent.shadowMapUniformBufferObject };

		DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Spotlight Shadow Descriptor Set";
		descriptorSetCreateInfo.layout = spotLightComponent.shadowMapDescriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = 1;
		descriptorSetCreateInfo.bindings = &lightUboBinding;
		spotLightComponent.shadowMapDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}
