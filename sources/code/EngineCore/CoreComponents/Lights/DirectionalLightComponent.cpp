#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <Common/Graphics/DescriptorSet.hpp>
#include <Common/Graphics/DescriptorSetLayout.hpp>
#include "DirectionalLightComponent.hpp"

using namespace Grindstone;

REFLECT_STRUCT_BEGIN(DirectionalLightComponent)
	REFLECT_STRUCT_MEMBER(color)
	REFLECT_STRUCT_MEMBER(sourceRadius)
	REFLECT_STRUCT_MEMBER(intensity)
	REFLECT_STRUCT_MEMBER(shadowResolution)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::SetupDirectionalLightComponent(entt::registry& registry, entt::entity entity) {
	auto& engineCore = EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	DirectionalLightComponent& directionalLightComponent = registry.get<DirectionalLightComponent>(entity);

	uint32_t shadowResolution = static_cast<uint32_t>(directionalLightComponent.shadowResolution);

	GraphicsAPI::RenderPass::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.debugName = "Directional Shadow Render Pass";
	renderPassCreateInfo.colorAttachments = nullptr;
	renderPassCreateInfo.colorAttachmentCount = 0;
	renderPassCreateInfo.depthFormat = GraphicsAPI::Format::D32_SFLOAT;
	directionalLightComponent.renderPass = graphicsCore->CreateRenderPass(renderPassCreateInfo);

	GraphicsAPI::Image::CreateInfo shadowMapDepthImageCreateInfo;
	shadowMapDepthImageCreateInfo.debugName = "Directional Shadow Map Depth Image";
	shadowMapDepthImageCreateInfo.format = renderPassCreateInfo.depthFormat;
	shadowMapDepthImageCreateInfo.width = shadowMapDepthImageCreateInfo.height = shadowResolution;
	shadowMapDepthImageCreateInfo.imageUsage =
		GraphicsAPI::ImageUsageFlags::DepthStencil |
		GraphicsAPI::ImageUsageFlags::Sampled;
	directionalLightComponent.depthTarget = graphicsCore->CreateImage(shadowMapDepthImageCreateInfo);

	GraphicsAPI::Framebuffer::CreateInfo shadowMapCreateInfo{};
	shadowMapCreateInfo.debugName = "Directional Shadow Framebuffer";
	shadowMapCreateInfo.width = shadowResolution;
	shadowMapCreateInfo.height = shadowResolution;
	shadowMapCreateInfo.renderPass = directionalLightComponent.renderPass;
	shadowMapCreateInfo.renderTargets = nullptr;
	shadowMapCreateInfo.renderTargetCount = 0;
	shadowMapCreateInfo.depthTarget = directionalLightComponent.depthTarget;
	directionalLightComponent.framebuffer = graphicsCore->CreateFramebuffer(shadowMapCreateInfo);

	{
		DirectionalLightComponent::UniformStruct lightStruct{};
		GraphicsAPI::Buffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "LightUbo";
		lightUniformBufferObjectCi.content = &lightStruct;
		lightUniformBufferObjectCi.bufferUsage = GraphicsAPI::BufferUsage::Uniform;
		lightUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemUsage::CPUToGPU;
		lightUniformBufferObjectCi.bufferSize = sizeof(DirectionalLightComponent::UniformStruct);
		directionalLightComponent.uniformBufferObject = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);

		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 2> lightLayoutBindings{};
		lightLayoutBindings[0].bindingId = 0;
		lightLayoutBindings[0].count = 1;
		lightLayoutBindings[0].type = GraphicsAPI::BindingType::UniformBuffer;
		lightLayoutBindings[0].stages = GraphicsAPI::ShaderStageBit::Fragment;

		lightLayoutBindings[1].bindingId = 1;
		lightLayoutBindings[1].count = 1;
		lightLayoutBindings[1].type = GraphicsAPI::BindingType::SampledImage;
		lightLayoutBindings[1].stages = GraphicsAPI::ShaderStageBit::Fragment;

		GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Directional Light Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(lightLayoutBindings.size());
		descriptorSetLayoutCreateInfo.bindings = lightLayoutBindings.data();
		directionalLightComponent.descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		std::array<GraphicsAPI::DescriptorSet::Binding, 2> lightBindings{
			directionalLightComponent.uniformBufferObject,
			directionalLightComponent.depthTarget
		};

		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Directional Light Descriptor Set";
		descriptorSetCreateInfo.layout = directionalLightComponent.descriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(lightBindings.size());
		descriptorSetCreateInfo.bindings = lightBindings.data();
		directionalLightComponent.descriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	{
		GraphicsAPI::Buffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "Directional Light Shadow Map";
		lightUniformBufferObjectCi.bufferUsage = GraphicsAPI::BufferUsage::Uniform;
		lightUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemUsage::CPUToGPU;
		lightUniformBufferObjectCi.bufferSize = sizeof(glm::mat4);
		directionalLightComponent.shadowMapUniformBufferObject = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);

		GraphicsAPI::DescriptorSetLayout::Binding lightUboBindingLayout{};
		lightUboBindingLayout.bindingId = 0;
		lightUboBindingLayout.count = 1;
		lightUboBindingLayout.type = GraphicsAPI::BindingType::UniformBuffer;
		lightUboBindingLayout.stages = GraphicsAPI::ShaderStageBit::Vertex;

		GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Directional Light Shadow Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = 1;
		descriptorSetLayoutCreateInfo.bindings = &lightUboBindingLayout;
		directionalLightComponent.shadowMapDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		GraphicsAPI::DescriptorSet::Binding lightUboBinding{ directionalLightComponent.shadowMapUniformBufferObject };

		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Directional Light Shadow Descriptor Set";
		descriptorSetCreateInfo.layout = directionalLightComponent.shadowMapDescriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = 1;
		descriptorSetCreateInfo.bindings = &lightUboBinding;
		directionalLightComponent.shadowMapDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}

void Grindstone::DestroyDirectionalLightComponent(entt::registry& registry, entt::entity entity) {
	auto& engineCore = EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	DirectionalLightComponent& directionalLightComponent = registry.get<DirectionalLightComponent>(entity);
	graphicsCore->DeleteDescriptorSet(directionalLightComponent.shadowMapDescriptorSet);
	graphicsCore->DeleteDescriptorSetLayout(directionalLightComponent.shadowMapDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSet(directionalLightComponent.descriptorSet);
	graphicsCore->DeleteDescriptorSetLayout(directionalLightComponent.descriptorSetLayout);
	graphicsCore->DeleteBuffer(directionalLightComponent.shadowMapUniformBufferObject);
	graphicsCore->DeleteBuffer(directionalLightComponent.uniformBufferObject);
	graphicsCore->DeleteFramebuffer(directionalLightComponent.framebuffer);
	graphicsCore->DeleteRenderPass(directionalLightComponent.renderPass);
	graphicsCore->DeleteImage(directionalLightComponent.depthTarget);
}
