#include <EngineCore/Reflection/ComponentReflection.hpp>
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

void Grindstone::SetupPointLightComponent(entt::registry& registry, entt::entity entity) {
	auto& engineCore = EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	PointLightComponent& pointLightComponent = registry.get<PointLightComponent>(entity);

	/* TODO: Re-add this when you come back to point light shadows
	uint32_t shadowResolution = static_cast<uint32_t>(pointLightComponent.shadowResolution);

	RenderPass::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.width = shadowResolution;
	renderPassCreateInfo.height = shadowResolution;
	renderPassCreateInfo.colorFormats = nullptr;
	renderPassCreateInfo.colorFormatCount = 0;
	renderPassCreateInfo.depthFormat = DepthFormat::D32;
	pointLightComponent.renderPass = graphicsCore->CreateRenderPass(renderPassCreateInfo);

	DepthStencilTarget::CreateInfo shadowMapDepthImageCreateInfo(renderPassCreateInfo.depthFormat, shadowResolution, shadowResolution, true, true, true, "Point Shadow Map Depth Image");
	pointLightComponent.depthTarget = graphicsCore->CreateDepthStencilTarget(shadowMapDepthImageCreateInfo);

	Framebuffer::CreateInfo shadowMapCreateInfo{};
	shadowMapCreateInfo.debugName = "Pointlight Shadow Framebuffer";
	shadowMapCreateInfo.renderPass = pointLightComponent.renderPass;
	shadowMapCreateInfo.renderTargetLists = nullptr;
	shadowMapCreateInfo.numRenderTargetLists = 0;
	shadowMapCreateInfo.depthTarget = pointLightComponent.depthTarget;
	shadowMapCreateInfo.isCubemap = true;
	pointLightComponent.framebuffer = graphicsCore->CreateFramebuffer(shadowMapCreateInfo);
	*/

	{
		PointLightComponent::UniformStruct lightInfoStruct{};
		Buffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "LightUbo";
		lightUniformBufferObjectCi.content = &lightInfoStruct;
		lightUniformBufferObjectCi.bufferUsage = BufferUsage::Uniform;
		lightUniformBufferObjectCi.memoryUsage = MemUsage::CPUToGPU;
		lightUniformBufferObjectCi.bufferSize = sizeof(PointLightComponent::UniformStruct);
		pointLightComponent.uniformBufferObject = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);
	}

	{
		std::array<DescriptorSetLayout::Binding, 1> lightBindings{};
		lightBindings[0].bindingId = 0;
		lightBindings[0].count = 1;
		lightBindings[0].type = BindingType::UniformBuffer;
		lightBindings[0].stages = ShaderStageBit::Fragment;

		/* TODO: Re-add this when you come back to point light shadows
		lightBindings[1].bindingId = 1;
		lightBindings[1].count = 1;
		lightBindings[1].type = BindingType::DepthTexture;
		lightBindings[1].stages = ShaderStageBit::Fragment;
		*/

		DescriptorSetLayout::CreateInfo pointLightDescriptorSetLayoutCreateInfo{};
		pointLightDescriptorSetLayoutCreateInfo.debugName = "Light UBO Descriptor Set Layout";
		pointLightDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(lightBindings.size());
		pointLightDescriptorSetLayoutCreateInfo.bindings = lightBindings.data();
		pointLightComponent.descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(pointLightDescriptorSetLayoutCreateInfo);
	}

	{
		std::array<DescriptorSet::Binding, 1> lightBindings;
		lightBindings[0] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(pointLightComponent.uniformBufferObject);

		/* TODO: Re-add this when you come back to point light shadows
		lightBindings[1].bindingIndex = 1;
		lightBindings[1].count = 1;
		lightBindings[1].bindingType = BindingType::DepthTexture;
		lightBindings[1].itemPtr = pointLightComponent.depthTarget;
		*/

		DescriptorSet::CreateInfo pointLightDescriptorSetCreateInfo{};
		pointLightDescriptorSetCreateInfo.debugName = "Light UBO Descriptor Set";
		pointLightDescriptorSetCreateInfo.layout = pointLightComponent.descriptorSetLayout;
		pointLightDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(lightBindings.size());
		pointLightDescriptorSetCreateInfo.bindings = lightBindings.data();
		pointLightComponent.descriptorSet = graphicsCore->CreateDescriptorSet(pointLightDescriptorSetCreateInfo);
	}

	/* TODO: Re-add this when you come back to point light shadows
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
	*/
}

void Grindstone::DestroyPointLightComponent(entt::registry& registry, entt::entity entity) {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	PointLightComponent& pointLightComponent = registry.get<PointLightComponent>(entity);
	graphicsCore->DeleteDescriptorSet(pointLightComponent.descriptorSet);
	graphicsCore->DeleteDescriptorSetLayout(pointLightComponent.descriptorSetLayout);
	graphicsCore->DeleteBuffer(pointLightComponent.uniformBufferObject);
}
