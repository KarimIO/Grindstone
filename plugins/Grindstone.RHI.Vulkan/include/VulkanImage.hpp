#pragma once

#include <Common/Graphics/Image.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class Image : public Grindstone::GraphicsAPI::Image {
	public:
		Image(VkImage image, VkFormat format, uint32_t swapchainIndex);
		Image(const CreateInfo& createInfo);
		void Create();
		virtual ~Image();
	public:
		virtual VkImage GetImage() const;
		virtual VkImageView GetImageView() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		uint32_t GetDepth() const;
		uint32_t GetMipLevels() const;
		uint32_t GetArrayLayers() const;
		VkImageAspectFlags GetAspect() const;
		void UpdateNativeImage(VkImage image, VkImageView imageView, VkFormat format);
		virtual void GenerateMipmaps(VkCommandBuffer cmd, VkImage image);
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void UploadData(
			const char* data,
			uint64_t dataSize
		) override;
		virtual void UploadDataRegions(void* buffer, size_t bufferSize, ImageRegion* regions, uint32_t regionCount) override;
		virtual void* MapMemory(uint64_t dataSize = MAPPED_MEMORY_ENTIRE_BUFFER, uint64_t dataOffset = 0) override;
		virtual void UnmapMemory() override;
		virtual Grindstone::Buffer ReadbackMemory() override;
	private:
		void CreateImage();

		std::string imageName;
		VkImageAspectFlags aspect;
		VkImageViewType imageViewType;
		VkDeviceMemory imageMemory = nullptr;
		VkImageView imageView = nullptr;
		VkImage image = nullptr;
		VkFormat vkFormat = VK_FORMAT_UNDEFINED;
	};
}
