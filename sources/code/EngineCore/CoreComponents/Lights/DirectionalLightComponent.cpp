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

	uint32_t shadowResolution = static_cast<uint32_t>(directionalLightComponent.shadowResolution);

	RenderPass::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.width = shadowResolution;
	renderPassCreateInfo.height = shadowResolution;
	renderPassCreateInfo.colorFormats = nullptr;
	renderPassCreateInfo.colorFormatCount = 0;
	renderPassCreateInfo.depthFormat = DepthFormat::D32;
	directionalLightComponent.renderPass = graphicsCore->CreateRenderPass(renderPassCreateInfo);

	DepthTarget::CreateInfo gbufferDepthImageCreateInfo(renderPassCreateInfo.depthFormat, shadowResolution, shadowResolution, false, false, true, "Directional Shadow Map Depth Image");
	directionalLightComponent.depthTarget = graphicsCore->CreateDepthTarget(gbufferDepthImageCreateInfo);

	Framebuffer::CreateInfo gbufferCreateInfo{};
	gbufferCreateInfo.debugName = "Directional Shadow Framebuffer";
	gbufferCreateInfo.renderPass = directionalLightComponent.renderPass;
	gbufferCreateInfo.renderTargetLists = nullptr;
	gbufferCreateInfo.numRenderTargetLists = 0;
	gbufferCreateInfo.depthTarget = directionalLightComponent.depthTarget;
	directionalLightComponent.framebuffer = graphicsCore->CreateFramebuffer(gbufferCreateInfo);

	{
		UniformBuffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "LightUbo";
		lightUniformBufferObjectCi.isDynamic = true;
		lightUniformBufferObjectCi.size = sizeof(DirectionalLightComponent::UniformStruct);
		directionalLightComponent.uniformBufferObject = graphicsCore->CreateUniformBuffer(lightUniformBufferObjectCi);

		DirectionalLightComponent::UniformStruct lightStruct{};
		directionalLightComponent.uniformBufferObject->UpdateBuffer(&lightStruct);

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
		descriptorSetLayoutCreateInfo.debugName = "Directional Light Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(lightLayoutBindings.size());
		descriptorSetLayoutCreateInfo.bindings = lightLayoutBindings.data();
		directionalLightComponent.descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		std::array<DescriptorSet::Binding, 2> lightBindings{};
		lightBindings[0].bindingIndex = 0;
		lightBindings[0].count = 1;
		lightBindings[0].bindingType = BindingType::UniformBuffer;
		lightBindings[0].itemPtr = directionalLightComponent.uniformBufferObject;

		lightBindings[1].bindingIndex = 1;
		lightBindings[1].count = 1;
		lightBindings[1].bindingType = BindingType::DepthTexture;
		lightBindings[1].itemPtr = directionalLightComponent.depthTarget;

		DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Directional Light Descriptor Set";
		descriptorSetCreateInfo.layout = directionalLightComponent.descriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(lightBindings.size());
		descriptorSetCreateInfo.bindings = lightBindings.data();
		directionalLightComponent.descriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	{
		UniformBuffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "Directional Light Shadow Map";
		lightUniformBufferObjectCi.isDynamic = true;
		lightUniformBufferObjectCi.size = sizeof(glm::mat4);
		directionalLightComponent.shadowMapUniformBufferObject = graphicsCore->CreateUniformBuffer(lightUniformBufferObjectCi);

		DescriptorSetLayout::Binding lightUboBindingLayout{};
		lightUboBindingLayout.bindingId = 0;
		lightUboBindingLayout.count = 1;
		lightUboBindingLayout.type = BindingType::UniformBuffer;
		lightUboBindingLayout.stages = ShaderStageBit::Vertex;

		DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Directional Light Shadow Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = 1;
		descriptorSetLayoutCreateInfo.bindings = &lightUboBindingLayout;
		directionalLightComponent.shadowMapDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		DescriptorSet::Binding lightUboBinding{};
		lightUboBinding.bindingIndex = 0;
		lightUboBinding.count = 1;
		lightUboBinding.bindingType = BindingType::UniformBuffer;
		lightUboBinding.itemPtr = directionalLightComponent.shadowMapUniformBufferObject;

		DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Directional Light Shadow Descriptor Set";
		descriptorSetCreateInfo.layout = directionalLightComponent.shadowMapDescriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = 1;
		descriptorSetCreateInfo.bindings = &lightUboBinding;
		directionalLightComponent.shadowMapDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}
