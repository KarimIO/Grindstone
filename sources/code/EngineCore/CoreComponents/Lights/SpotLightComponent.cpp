#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <Common/Graphics/DescriptorSet.hpp>
#include <Common/Graphics/DescriptorSetLayout.hpp>
#include "SpotLightComponent.hpp"

using namespace Grindstone;

REFLECT_STRUCT_BEGIN(SpotLightComponent)
	REFLECT_STRUCT_MEMBER(color)
	REFLECT_STRUCT_MEMBER(attenuationRadius)
	REFLECT_STRUCT_MEMBER(intensity)
	REFLECT_STRUCT_MEMBER(innerAngle)
	REFLECT_STRUCT_MEMBER(outerAngle)
	REFLECT_STRUCT_MEMBER(shadowResolution)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::SetupSpotLightComponent(entt::registry& registry, entt::entity entity) {
	auto& engineCore = EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	SpotLightComponent& spotLightComponent = registry.get<SpotLightComponent>(entity);

	uint32_t shadowResolution = static_cast<uint32_t>(spotLightComponent.shadowResolution);

	GraphicsAPI::RenderPass::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.debugName = "Spotlight Shadow Render Pass";
	renderPassCreateInfo.colorAttachments = nullptr;
	renderPassCreateInfo.colorAttachmentCount = 0;
	renderPassCreateInfo.depthFormat = GraphicsAPI::Format::D32_SFLOAT;
	spotLightComponent.renderPass = graphicsCore->CreateRenderPass(renderPassCreateInfo);

	GraphicsAPI::Image::CreateInfo shadowMapDepthImageCreateInfo{};
	shadowMapDepthImageCreateInfo.debugName = "Spot Shadow Map Depth Image";
	shadowMapDepthImageCreateInfo.format = renderPassCreateInfo.depthFormat;
	shadowMapDepthImageCreateInfo.width = shadowMapDepthImageCreateInfo.height = shadowResolution;
	shadowMapDepthImageCreateInfo.imageUsage =
		GraphicsAPI::ImageUsageFlags::DepthStencil |
		GraphicsAPI::ImageUsageFlags::Sampled;
	spotLightComponent.depthTarget = graphicsCore->CreateImage(shadowMapDepthImageCreateInfo);

	GraphicsAPI::Framebuffer::CreateInfo shadowMapCreateInfo{};
	shadowMapCreateInfo.debugName = "Spotlight Shadow Framebuffer";
	shadowMapCreateInfo.width = shadowResolution;
	shadowMapCreateInfo.height = shadowResolution;
	shadowMapCreateInfo.renderPass = spotLightComponent.renderPass;
	shadowMapCreateInfo.renderTargets = nullptr;
	shadowMapCreateInfo.renderTargetCount = 0;
	shadowMapCreateInfo.depthTarget = spotLightComponent.depthTarget;
	spotLightComponent.framebuffer = graphicsCore->CreateFramebuffer(shadowMapCreateInfo);

	{
		SpotLightComponent::UniformStruct lightStruct{};
		GraphicsAPI::Buffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.content = &lightStruct;
		lightUniformBufferObjectCi.debugName = "LightUbo";
		lightUniformBufferObjectCi.bufferUsage = GraphicsAPI::BufferUsage::Uniform;
		lightUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemUsage::CPUToGPU;
		lightUniformBufferObjectCi.bufferSize = sizeof(SpotLightComponent::UniformStruct);
		spotLightComponent.uniformBufferObject = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);

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
		descriptorSetLayoutCreateInfo.debugName = "Spotlight Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(lightLayoutBindings.size());
		descriptorSetLayoutCreateInfo.bindings = lightLayoutBindings.data();
		spotLightComponent.descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		std::array<GraphicsAPI::DescriptorSet::Binding, 2> lightBindings{
			GraphicsAPI::DescriptorSet::Binding::UniformBuffer( spotLightComponent.uniformBufferObject ),
			GraphicsAPI::DescriptorSet::Binding::SampledImage( spotLightComponent.depthTarget )
		};

		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Spotlight Descriptor Set";
		descriptorSetCreateInfo.layout = spotLightComponent.descriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(lightBindings.size());
		descriptorSetCreateInfo.bindings = lightBindings.data();
		spotLightComponent.descriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	{
		GraphicsAPI::Buffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "Spot Shadow Map";
		lightUniformBufferObjectCi.bufferUsage = GraphicsAPI::BufferUsage::Uniform;
		lightUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemUsage::CPUToGPU;
		lightUniformBufferObjectCi.bufferSize = sizeof(glm::mat4);
		spotLightComponent.shadowMapUniformBufferObject = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);

		GraphicsAPI::DescriptorSetLayout::Binding lightUboBindingLayout{};
		lightUboBindingLayout.bindingId = 0;
		lightUboBindingLayout.count = 1;
		lightUboBindingLayout.type = GraphicsAPI::BindingType::UniformBuffer;
		lightUboBindingLayout.stages = GraphicsAPI::ShaderStageBit::Vertex;

		GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.debugName = "Spotlight Shadow Descriptor Set Layout";
		descriptorSetLayoutCreateInfo.bindingCount = 1;
		descriptorSetLayoutCreateInfo.bindings = &lightUboBindingLayout;
		spotLightComponent.shadowMapDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		GraphicsAPI::DescriptorSet::Binding lightUboBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( spotLightComponent.shadowMapUniformBufferObject );

		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
		descriptorSetCreateInfo.debugName = "Spotlight Shadow Descriptor Set";
		descriptorSetCreateInfo.layout = spotLightComponent.shadowMapDescriptorSetLayout;
		descriptorSetCreateInfo.bindingCount = 1;
		descriptorSetCreateInfo.bindings = &lightUboBinding;
		spotLightComponent.shadowMapDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}

void Grindstone::DestroySpotLightComponent(entt::registry& registry, entt::entity entity) {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	SpotLightComponent& spotLightComponent = registry.get<SpotLightComponent>(entity);
	graphicsCore->DeleteDescriptorSet(spotLightComponent.shadowMapDescriptorSet);
	graphicsCore->DeleteDescriptorSetLayout(spotLightComponent.shadowMapDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSet(spotLightComponent.descriptorSet);
	graphicsCore->DeleteDescriptorSetLayout(spotLightComponent.descriptorSetLayout);
	graphicsCore->DeleteBuffer(spotLightComponent.shadowMapUniformBufferObject);
	graphicsCore->DeleteBuffer(spotLightComponent.uniformBufferObject);
	graphicsCore->DeleteFramebuffer(spotLightComponent.framebuffer);
	graphicsCore->DeleteRenderPass(spotLightComponent.renderPass);
	graphicsCore->DeleteImage(spotLightComponent.depthTarget);
}
