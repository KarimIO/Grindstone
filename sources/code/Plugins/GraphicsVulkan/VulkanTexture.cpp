#include <assert.h>
#include "VulkanTexture.hpp"
#include "VulkanUtils.hpp"
#include "VulkanFormat.hpp"
#include "VulkanCore.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanTextureBindingLayout::VulkanTextureBindingLayout(TextureBindingLayout::CreateInfo& ci) {
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

			if (vkCreateDescriptorSetLayout(VulkanCore::Get().GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}
		}

		VulkanTextureBindingLayout::~VulkanTextureBindingLayout() {
			vkDestroyDescriptorSetLayout(VulkanCore::Get().GetDevice(), descriptorSetLayout, nullptr);
		}

		VkDescriptorSetLayout VulkanTextureBindingLayout::GetDescriptorSetLayout() {
			return descriptorSetLayout;
		}

		VulkanTexture::VulkanTexture(Texture::CreateInfo& ci) {
			uint32_t mipLevels;
			CreateTextureImage(ci, mipLevels);
			imageView = CreateImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
			CreateTextureSampler(ci, mipLevels);
		}

		VulkanTexture::~VulkanTexture() {
			VkDevice device = VulkanCore::Get().GetDevice();
			vkDestroySampler(device, sampler, nullptr);
			vkDestroyImageView(device, imageView, nullptr);

			vkDestroyImage(device, image, nullptr);
			vkFreeMemory(device, imageMemory, nullptr);
		}

		void VulkanTexture::CreateTextureImage(Texture::CreateInfo& ci, uint32_t &mipLevels) {
			VkDevice device = VulkanCore::Get().GetDevice();

			uint8_t channels = 4;
			format = VK_FORMAT_R8G8B8A8_UNORM;
			//TranslateColorFormatToVulkan(ci.format, channels);

			mipLevels = std::floor(std::log2((ci.width > ci.height) ? ci.width : ci.height));

			uint32_t imageSize = ci.width * ci.height * channels;

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, ci.data, static_cast<size_t>(imageSize));
			vkUnmapMemory(device, stagingBufferMemory);

			CreateImage(ci.width, ci.height, mipLevels, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

			TransitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
			CopyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(ci.width), static_cast<uint32_t>(ci.height));
			TransitionImageLayout(image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);

			//if (ci.options.generate_mipmaps)
			//	generateMipmaps(image_, VK_FORMAT_R8G8B8A8_UNORM, ci.width, ci.height, mipLevels);
		}

		void VulkanTexture::CreateTextureSampler(Texture::CreateInfo &ci, uint32_t mipLevels) {
			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = TranslateFilterToVulkan(ci.options.magFilter);
			samplerInfo.minFilter = TranslateFilterToVulkan(ci.options.minFilter);
			samplerInfo.addressModeU = TranslateWrapToVulkan(ci.options.wrapModeU);
			samplerInfo.addressModeV = TranslateWrapToVulkan(ci.options.wrapModeV);
			samplerInfo.addressModeW = TranslateWrapToVulkan(ci.options.wrapModeW);
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

			if (vkCreateSampler(VulkanCore::Get().GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture sampler!");
			}
		}

		void VulkanTexture::RecreateTexture(CreateInfo& createInfo) {
			std::cout << "VulkanTexture::RecreateTexture is not used\n";
			assert(false);
		}

		VkImageView VulkanTexture::GetImageView() {
			return imageView;
		}

		VkSampler VulkanTexture::GetSampler() {
			return sampler;
		}

		VulkanTextureBinding::VulkanTextureBinding(TextureBinding::CreateInfo& ci) {
			VkDescriptorSetLayout layouts = ((VulkanTextureBindingLayout *)ci.layout)->GetDescriptorSetLayout();

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = VulkanCore::Get().descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &layouts;

			if (vkAllocateDescriptorSets(VulkanCore::Get().GetDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			VulkanTexture *tex = (VulkanTexture *)ci.textures->texture;
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = tex->GetImageView();
			imageInfo.sampler = tex->GetSampler();

			VkWriteDescriptorSet descriptorWrites = {};
			descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites.dstSet = descriptorSet;
			descriptorWrites.dstBinding = 0;
			descriptorWrites.dstArrayElement = 0;
			descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites.descriptorCount = 1;
			descriptorWrites.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(VulkanCore::Get().GetDevice(), 1, &descriptorWrites, 0, nullptr);
		}

		VulkanTextureBinding::~VulkanTextureBinding() {
		}

		VkDescriptorSet VulkanTextureBinding::GetDescriptorSet() {
			return descriptorSet;
		}
	};
};
