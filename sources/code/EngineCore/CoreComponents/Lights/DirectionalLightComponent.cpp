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
		lightUniformBufferObjectCi.content = &lightStruct;
		lightUniformBufferObjectCi.bufferUsage = GraphicsAPI::BufferUsage::Uniform;
		lightUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
		lightUniformBufferObjectCi.bufferSize = sizeof(DirectionalLightComponent::UniformStruct);

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


		GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{
			.debugName = "Directional Light Descriptor Set",
			.layout = directionalLightComponent.descriptorSetLayout,
			.bindingCount = 1u,
		};

		for (size_t i = 0; i < FRAME_COUNT; ++i) {
			std::string debugName = std::format("Directional Light UBO [{}]", i);
			lightUniformBufferObjectCi.debugName = debugName.c_str();
			directionalLightComponent.uniformBufferObject[i] = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);
			GraphicsAPI::DescriptorSet::Binding lightBindings = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(directionalLightComponent.uniformBufferObject[i]);
			descriptorSetCreateInfo.bindings = &lightBindings;
			directionalLightComponent.descriptorSet[i] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
		}
	}

	{
		GraphicsAPI::Buffer::CreateInfo lightUniformBufferObjectCi{
			.bufferSize = sizeof(glm::mat4),
			.bufferUsage = GraphicsAPI::BufferUsage::Uniform,
			.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU,
		};

		for (size_t i = 0; i < FRAME_COUNT; ++i) {
			for (size_t j = 0; j < MAX_CASCADE_COUNT; ++j) {
				std::string debugName = std::format("Directional Light Shadow Map {}[{}]", i, j);
				lightUniformBufferObjectCi.debugName = debugName.c_str();
				directionalLightComponent.shadowMapUniformBufferObjects[i][j] = graphicsCore->CreateBuffer(lightUniformBufferObjectCi);
			}
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

		for (size_t i = 0; i < FRAME_COUNT; ++i) {
			for (size_t j = 0; j < MAX_CASCADE_COUNT; ++j) {
				GraphicsAPI::DescriptorSet::Binding lightUboBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(directionalLightComponent.shadowMapUniformBufferObjects[i][j]);
				std::string debugName = std::format("Directional Light Shadow Descriptor Set Cascade {}[{}]", i, j);
				descriptorSetCreateInfo.bindings = &lightUboBinding;
				descriptorSetCreateInfo.debugName = debugName.c_str();
				directionalLightComponent.shadowMapDescriptorSets[i][j] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
			}
		}
	}
}

void Grindstone::DirectionalLightComponent::Destroy(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	DirectionalLightComponent& directionalLightComponent = cxtSet.GetEntityRegistry().get<DirectionalLightComponent>(entity);

	EngineCore::GetInstance().PushDeletion(
		[directionalLightComponent]() {
			EngineCore& engineCore = EngineCore::GetInstance();
			GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

			for (size_t i = 0; i < FRAME_COUNT; ++i) {
				for (size_t j = 0; j < MAX_CASCADE_COUNT; ++j) {
					graphicsCore->DeleteDescriptorSet(directionalLightComponent.shadowMapDescriptorSets[i][j]);
					graphicsCore->DeleteBuffer(directionalLightComponent.shadowMapUniformBufferObjects[i][j]);
				}

				graphicsCore->DeleteDescriptorSet(directionalLightComponent.descriptorSet[i]);
				graphicsCore->DeleteBuffer(directionalLightComponent.uniformBufferObject[i]);
			}

			graphicsCore->DeleteDescriptorSetLayout(directionalLightComponent.shadowMapDescriptorSetLayout);
			graphicsCore->DeleteDescriptorSetLayout(directionalLightComponent.descriptorSetLayout);
		}
	);

	for (size_t i = 0; i < FRAME_COUNT; ++i) {
		for (size_t j = 0; j < MAX_CASCADE_COUNT; ++j) {
			directionalLightComponent.shadowMapDescriptorSets[i][j] = nullptr;
			directionalLightComponent.shadowMapUniformBufferObjects[i][j] = nullptr;
		}

		directionalLightComponent.descriptorSet[i] = nullptr;
		directionalLightComponent.uniformBufferObject[i] = nullptr;
	}

	directionalLightComponent.shadowMapDescriptorSetLayout = nullptr;
	directionalLightComponent.descriptorSetLayout = nullptr;
}
