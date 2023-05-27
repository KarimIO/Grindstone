#include "VulkanUniformBuffer.hpp"
#include "VulkanCore.hpp"
#include "VulkanUtils.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanUniformBufferBinding::VulkanUniformBufferBinding(UniformBufferBinding::CreateInfo& createInfo) {
			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = 0;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.pImmutableSamplers = nullptr;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			VkDescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &uboLayoutBinding;

			if (vkCreateDescriptorSetLayout(VulkanCore::Get().GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}
		}

		VulkanUniformBufferBinding::~VulkanUniformBufferBinding() {
			vkDestroyDescriptorSetLayout(VulkanCore::Get().GetDevice(), descriptorSetLayout, nullptr);
		}

		VkDescriptorSetLayout VulkanUniformBufferBinding::GetDescriptorSetLayout() {
			return descriptorSetLayout;
		}


		//==========================


		VulkanUniformBuffer::VulkanUniformBuffer(UniformBuffer::CreateInfo& createInfo) {
			size = createInfo.size;
			CreateBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, memory);
			
			VkDescriptorSetLayout layouts = static_cast<VulkanUniformBufferBinding*>(createInfo.binding)->GetDescriptorSetLayout();

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = VulkanCore::Get().descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &layouts;

			if (vkAllocateDescriptorSets(VulkanCore::Get().GetDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = createInfo.size;

			VkWriteDescriptorSet descriptorWrites = {};
			descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites.dstSet = descriptorSet;
			descriptorWrites.dstBinding = 0;
			descriptorWrites.dstArrayElement = 0;
			descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites.descriptorCount = 1;
			descriptorWrites.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(VulkanCore::Get().GetDevice(), 1, &descriptorWrites, 0, nullptr);
		}

		VulkanUniformBuffer::~VulkanUniformBuffer() {
		}

		VkDescriptorSet VulkanUniformBuffer::GetDescriptorSet() {
			return descriptorSet;
		}
		
		void VulkanUniformBuffer::UpdateBuffer(void * content) {
			VkDevice device = VulkanCore::Get().GetDevice();
			void* data;
			vkMapMemory(device, memory, 0, size, 0, &data);
			memcpy(data, content, size);
			vkUnmapMemory(device, memory);
		};
		
		void VulkanUniformBuffer::Bind() {
		};
		
	};
};
