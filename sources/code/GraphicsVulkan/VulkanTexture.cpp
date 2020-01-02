#include "VulkanTexture.hpp"
#include "VulkanUtils.hpp"
#include "VulkanFormat.hpp"
#include "VulkanGraphicsWrapper.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanTextureBindingLayout::VulkanTextureBindingLayout(TextureBindingLayoutCreateInfo ci) {
			VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
			samplerLayoutBinding.binding = 0;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerLayoutBinding.pImmutableSamplers = nullptr;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &samplerLayoutBinding;

			if (vkCreateDescriptorSetLayout(VulkanGraphicsWrapper::get().getDevice(), &layoutInfo, nullptr, &descriptor_set_layout_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}
		}

		VulkanTextureBindingLayout::~VulkanTextureBindingLayout() {
			vkDestroyDescriptorSetLayout(VulkanGraphicsWrapper::get().getDevice(), descriptor_set_layout_, nullptr);
		}

		VkDescriptorSetLayout VulkanTextureBindingLayout::getDescriptorSetLayout() {
			return descriptor_set_layout_;
		}

		VulkanTexture::VulkanTexture(TextureCreateInfo ci) {
			uint32_t mipLevels;
			createTextureImage(ci, mipLevels);
			image_view_ = createImageView(image_, format_, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
			createTextureSampler(ci, mipLevels);
		}

		VulkanTexture::~VulkanTexture() {
			VkDevice device = VulkanGraphicsWrapper::get().getDevice();
			vkDestroySampler(device, sampler_, nullptr);
			vkDestroyImageView(device, image_view_, nullptr);

			vkDestroyImage(device, image_, nullptr);
			vkFreeMemory(device, image_memory_, nullptr);
		}

		void VulkanTexture::createTextureImage(TextureCreateInfo &ci, uint32_t &mipLevels) {
			VkDevice device = VulkanGraphicsWrapper::get().getDevice();

			uint8_t channels = 4;
			format_ = VK_FORMAT_R8G8B8A8_UNORM;
			//TranslateColorFormatToVulkan(ci.format, channels);

			mipLevels = std::floor(std::log2((ci.width > ci.height) ? ci.width : ci.height));

			uint32_t imageSize = ci.width * ci.height * channels;

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, ci.data, static_cast<size_t>(imageSize));
			vkUnmapMemory(device, stagingBufferMemory);

			createImage(ci.width, ci.height, mipLevels, format_, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image_, image_memory_);

			transitionImageLayout(image_, format_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
			copyBufferToImage(stagingBuffer, image_, static_cast<uint32_t>(ci.width), static_cast<uint32_t>(ci.height));
			//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);

			//if (ci.options.generate_mipmaps)
			//	generateMipmaps(image_, VK_FORMAT_R8G8B8A8_UNORM, ci.width, ci.height, mipLevels);
		}

		void VulkanTexture::createTextureSampler(TextureCreateInfo &ci, uint32_t mipLevels) {
			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = TranslateFilterToVulkan(ci.options.mag_filter);
			samplerInfo.minFilter = TranslateFilterToVulkan(ci.options.min_filter);
			samplerInfo.addressModeU = TranslateWrapToVulkan(ci.options.wrap_mode_u);
			samplerInfo.addressModeV = TranslateWrapToVulkan(ci.options.wrap_mode_v);
			samplerInfo.addressModeW = TranslateWrapToVulkan(ci.options.wrap_mode_w);
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = 16;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.minLod = 0;
			samplerInfo.maxLod = static_cast<float>(mipLevels);
			samplerInfo.mipLodBias = 0;

			if (vkCreateSampler(VulkanGraphicsWrapper::get().getDevice(), &samplerInfo, nullptr, &sampler_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture sampler!");
			}
		}

		VkImageView VulkanTexture::getImageView() {
			return image_view_;
		}

		VkSampler VulkanTexture::getSampler() {
			return sampler_;
		}

		VulkanTextureBinding::VulkanTextureBinding(TextureBindingCreateInfo ci) {
			VkDescriptorSetLayout layouts = ((VulkanTextureBindingLayout *)ci.layout)->getDescriptorSetLayout();

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = VulkanGraphicsWrapper::get().descriptor_pool_;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &layouts;

			if (vkAllocateDescriptorSets(VulkanGraphicsWrapper::get().getDevice(), &allocInfo, &descriptor_set_) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			VulkanTexture *tex = (VulkanTexture *)ci.textures->texture;
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = tex->getImageView();
			imageInfo.sampler = tex->getSampler();

			VkWriteDescriptorSet descriptorWrites = {};
			descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites.dstSet = descriptor_set_;
			descriptorWrites.dstBinding = 0;
			descriptorWrites.dstArrayElement = 0;
			descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites.descriptorCount = 1;
			descriptorWrites.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(VulkanGraphicsWrapper::get().getDevice(), 1, &descriptorWrites, 0, nullptr);
		}

		VulkanTextureBinding::~VulkanTextureBinding() {
		}

		VkDescriptorSet VulkanTextureBinding::getDescriptorSet() {
			return descriptor_set_;
		}
	};
};
