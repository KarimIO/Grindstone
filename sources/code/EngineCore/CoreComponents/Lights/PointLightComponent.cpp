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
	REFLECT_STRUCT_MEMBER(shadowResolution)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::SetupPointLightComponent(ECS::Entity& entity, void* componentPtr) {
	auto& engineCore = EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	PointLightComponent& pointLightComponent = *static_cast<PointLightComponent*>(componentPtr);

	uint32_t shadowResolution = static_cast<uint32_t>(pointLightComponent.shadowResolution);

	RenderPass::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.width = shadowResolution;
	renderPassCreateInfo.height = shadowResolution;
	renderPassCreateInfo.colorFormats = nullptr;
	renderPassCreateInfo.colorFormatCount = 0;
	renderPassCreateInfo.depthFormat = DepthFormat::D32;
	pointLightComponent.renderPass = graphicsCore->CreateRenderPass(renderPassCreateInfo);

	DepthTarget::CreateInfo shadowMapDepthImageCreateInfo(renderPassCreateInfo.depthFormat, shadowResolution, shadowResolution, true, true, true, "Point Shadow Map Depth Image");
	pointLightComponent.depthTarget = graphicsCore->CreateDepthTarget(shadowMapDepthImageCreateInfo);

	Framebuffer::CreateInfo shadowMapCreateInfo{};
	shadowMapCreateInfo.debugName = "Pointlight Shadow Framebuffer";
	shadowMapCreateInfo.renderPass = pointLightComponent.renderPass;
	shadowMapCreateInfo.renderTargetLists = nullptr;
	shadowMapCreateInfo.numRenderTargetLists = 0;
	shadowMapCreateInfo.depthTarget = pointLightComponent.depthTarget;
	pointLightComponent.framebuffer = graphicsCore->CreateFramebuffer(shadowMapCreateInfo);

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
		std::array<DescriptorSet::Binding, 2> lightBindings{};
		lightBindings[0].bindingIndex = 0;
		lightBindings[0].count = 1;
		lightBindings[0].bindingType = BindingType::UniformBuffer;
		lightBindings[0].itemPtr = pointLightComponent.uniformBufferObject;

		lightBindings[1].bindingIndex = 1;
		lightBindings[1].count = 1;
		lightBindings[1].bindingType = BindingType::DepthTexture;
		lightBindings[1].itemPtr = pointLightComponent.depthTarget;

		DescriptorSet::CreateInfo pointLightDescriptorSetCreateInfo{};
		pointLightDescriptorSetCreateInfo.debugName = "Light UBO Descriptor Set";
		pointLightDescriptorSetCreateInfo.layout = pointLightComponent.descriptorSetLayout;
		pointLightDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(lightBindings.size());
		pointLightDescriptorSetCreateInfo.bindings = lightBindings.data();
		pointLightComponent.descriptorSet = graphicsCore->CreateDescriptorSet(pointLightDescriptorSetCreateInfo);
	}

	{
		UniformBuffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "Point Shadow Map";
		lightUniformBufferObjectCi.isDynamic = true;
		lightUniformBufferObjectCi.size = sizeof(glm::mat4);
		pointLightComponent.shadowMapUniformBufferObject = graphicsCore->CreateUniformBuffer(lightUniformBufferObjectCi);

		DescriptorSetLayout::Binding lightUboBindingLayout{};
		lightUboBindingLayout.bindingId = 0;
		lightUboBindingLayout.count = 1;
		lightUboBindingLayout.type = BindingType::UniformBuffer;
		lightUboBindingLayout.stages = ShaderStageBit::Vertex;

		DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Pointlight Shadow Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = 1;
		descriptorSetLayoutCreateInfo.bindings = &lightUboBindingLayout;
		pointLightComponent.shadowMapDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		DescriptorSet::Binding lightUboBinding{};
		lightUboBinding.bindingIndex = 0;
		lightUboBinding.count = 1;
		lightUboBinding.bindingType = BindingType::UniformBuffer;
		lightUboBinding.itemPtr = pointLightComponent.shadowMapUniformBufferObject;

		DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Pointlight Shadow Descriptor Set";
		descriptorSetCreateInfo.layout = pointLightComponent.shadowMapDescriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = 1;
		descriptorSetCreateInfo.bindings = &lightUboBinding;
		pointLightComponent.shadowMapDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}
