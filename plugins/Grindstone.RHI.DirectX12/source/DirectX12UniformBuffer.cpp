#include "DirectX12UniformBuffer.hpp"
#include "DirectX12GraphicsWrapper.hpp"
#include "DirectX12Utils.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX12UniformBufferBinding::DirectX12UniformBufferBinding(UniformBufferBindingCreateInfo ci) {
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

			if (vkCreateDescriptorSetLayout(DirectX12GraphicsWrapper::get().getDevice(), &layoutInfo, nullptr, &descriptor_set_layout_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}
		}

		DirectX12UniformBufferBinding::~DirectX12UniformBufferBinding() {
			vkDestroyDescriptorSetLayout(DirectX12GraphicsWrapper::get().getDevice(), descriptor_set_layout_, nullptr);
		}

		VkDescriptorSetLayout DirectX12UniformBufferBinding::getDescriptorSetLayout() {
			return descriptor_set_layout_;
		}


		//==========================


		DirectX12UniformBuffer::DirectX12UniformBuffer(UniformBufferCreateInfo ci) {
			size_ = ci.size;
			createBuffer(size_, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer_, memory_);
			
			VkDescriptorSetLayout layouts = ((DirectX12UniformBufferBinding *)ci.binding)->getDescriptorSetLayout();

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = DirectX12GraphicsWrapper::get().descriptor_pool_;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &layouts;

			if (vkAllocateDescriptorSets(DirectX12GraphicsWrapper::get().getDevice(), &allocInfo, &descriptor_set_) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = buffer_;
			bufferInfo.offset = 0;
			bufferInfo.range = ci.size;

			VkWriteDescriptorSet descriptorWrites = {};
			descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites.dstSet = descriptor_set_;
			descriptorWrites.dstBinding = 0;
			descriptorWrites.dstArrayElement = 0;
			descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites.descriptorCount = 1;
			descriptorWrites.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(DirectX12GraphicsWrapper::get().getDevice(), 1, &descriptorWrites, 0, nullptr);
		}

		DirectX12UniformBuffer::~DirectX12UniformBuffer() {
		}

		VkDescriptorSet DirectX12UniformBuffer::getDescriptorSet() {
			return descriptor_set_;
		}
		
		void DirectX12UniformBuffer::UpdateUniformBuffer(void * content) {
			VkDevice device = DirectX12GraphicsWrapper::get().getDevice();
			void* data;
			vkMapMemory(device, memory_, 0, size_, 0, &data);
			memcpy(data, content, size_);
			vkUnmapMemory(device, memory_);
		};
		
		void DirectX12UniformBuffer::Bind() {
		};
		
	};
};
