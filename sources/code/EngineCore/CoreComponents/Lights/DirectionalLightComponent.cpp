#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/Buffer.hpp>
#include <Common/Graphics/DescriptorSet.hpp>
#include <Common/Graphics/DescriptorSetLayout.hpp>
#include "DirectionalLightComponent.hpp"

using namespace Grindstone;

REFLECT_STRUCT_BEGIN(DirectionalLightComponent)
	REFLECT_STRUCT_MEMBER(color)
	REFLECT_STRUCT_MEMBER(intensity)
	REFLECT_STRUCT_MEMBER(cascadeCount)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::DirectionalLightComponent::Construct(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	auto& engineCore = EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto eventDispatcher = engineCore.GetEventDispatcher();

	DirectionalLightComponent& directionalLightComponent = cxtSet.GetEntityRegistry().get<DirectionalLightComponent>(entity);

	{
		DirectionalLightComponent::UniformStruct lightStruct{};
		GraphicsAPI::Buffer::CreateInfo lightUniformBufferObjectCi{};
		lightUniformBufferObjectCi.debugName = "LightUbo";
		lightUniformBufferObjectCi.content = &lightStruct;
		lightUniformBufferObjectCi.bufferUsage = GraphicsAPI::BufferUsage::Uniform;
		lightUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
		lightUniformBufferObjectCi.bufferSize = sizeof(DirectionalLightComponent::UniformStruct);
		directionalLightComponent.uniformBufferObject = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);

		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 1> lightLayoutBindings{};
		lightLayoutBindings[0].bindingId = 0;
		lightLayoutBindings[0].count = 1;
		lightLayoutBindings[0].type = GraphicsAPI::BindingType::UniformBuffer;
		lightLayoutBindings[0].stages = GraphicsAPI::ShaderStageBit::Fragment;

		GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{
			.debugName = "Directional Light Descriptor Set Layout",
			.bindings = lightLayoutBindings.data(),
			.bindingCount = static_cast<uint32_t>(lightLayoutBindings.size())
		};
		directionalLightComponent.descriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(descriptorSetLayoutCreateInfo);

		std::array<GraphicsAPI::DescriptorSet::Binding, 1> lightBindings{
			GraphicsAPI::DescriptorSet::Binding::UniformBuffer( directionalLightComponent.uniformBufferObject )
		};

		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{
			.debugName = "Directional Light Descriptor Set",
			.layout = directionalLightComponent.descriptorSetLayout,
			.bindings = lightBindings.data(),
			.bindingCount = static_cast<uint32_t>(lightBindings.size()),
		};
		directionalLightComponent.descriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	{
		GraphicsAPI::Buffer::CreateInfo lightUniformBufferObjectCi{
			.bufferSize = sizeof(glm::mat4),
			.bufferUsage = GraphicsAPI::BufferUsage::Uniform,
			.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU,
		};

		for (size_t i = 0; i < MAX_CASCADE_COUNT; ++i) {
			std::string debugName = std::format("Directional Light Shadow Map {}", i);
			lightUniformBufferObjectCi.debugName = debugName.c_str();
			directionalLightComponent.shadowMapUniformBufferObjects[i] = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);
		}

		GraphicsAPI::DescriptorSetLayout::Binding lightUboBindingLayout{
			.bindingId = 0,
			.count = 1,
			.type = GraphicsAPI::BindingType::UniformBuffer,
			.stages = GraphicsAPI::ShaderStageBit::Vertex,
		};

		GraphicsAPI::DescriptorSetLayout::CreateInfo descriptorSetLayoutCreateInfo{
			.debugName = "Directional Light Shadow Descriptor Set Layout",
			.bindings = &lightUboBindingLayout,
			.bindingCount = 1
		};
		directionalLightComponent.shadowMapDescriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(descriptorSetLayoutCreateInfo);

		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{
			.layout = directionalLightComponent.shadowMapDescriptorSetLayout,
			.bindingCount = 1
		};
		for (size_t i = 0; i < MAX_CASCADE_COUNT; ++i) {
			GraphicsAPI::DescriptorSet::Binding lightUboBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(directionalLightComponent.shadowMapUniformBufferObjects[i]);

			std::string debugName = std::format("Directional Light Shadow Descriptor Set Cascade {}", i);
			descriptorSetCreateInfo.bindings = &lightUboBinding;
			descriptorSetCreateInfo.debugName = debugName.c_str();
			directionalLightComponent.shadowMapDescriptorSets[i] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
		}
	}
}

void Grindstone::DirectionalLightComponent::Destroy(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	DirectionalLightComponent& directionalLightComponent = cxtSet.GetEntityRegistry().get<DirectionalLightComponent>(entity);

	EngineCore::GetInstance().PushDeletion(
		[directionalLightComponent]() {
			EngineCore& engineCore = EngineCore::GetInstance();
			GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

			for (size_t i = 0; i < MAX_CASCADE_COUNT; ++i) {
				graphicsCore->DeleteDescriptorSet(directionalLightComponent.shadowMapDescriptorSets[i]);
				graphicsCore->DeleteBuffer(directionalLightComponent.shadowMapUniformBufferObjects[i]);
			}

			graphicsCore->DeleteDescriptorSetLayout(directionalLightComponent.shadowMapDescriptorSetLayout);
			graphicsCore->DeleteDescriptorSet(directionalLightComponent.descriptorSet);
			graphicsCore->DeleteDescriptorSetLayout(directionalLightComponent.descriptorSetLayout);
			graphicsCore->DeleteBuffer(directionalLightComponent.uniformBufferObject);
		}
	);

	for (size_t i = 0; i < MAX_CASCADE_COUNT; ++i) {
		directionalLightComponent.shadowMapDescriptorSets[i] = nullptr;
		directionalLightComponent.shadowMapUniformBufferObjects[i] = nullptr;
	}
	directionalLightComponent.shadowMapDescriptorSetLayout = nullptr;
	directionalLightComponent.descriptorSet = nullptr;
	directionalLightComponent.descriptorSetLayout = nullptr;
	directionalLightComponent.uniformBufferObject = nullptr;
}
