#pragma once

#include <Common/Graphics/Buffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class Buffer : public Grindstone::GraphicsAPI::Buffer {
	public:
		Buffer(const Grindstone::GraphicsAPI::Buffer::CreateInfo& createInfo);
		virtual ~Buffer() override;

		virtual void* Map() override;
		virtual void Unmap() override;
		virtual void UploadData(const void* data, size_t size, size_t offset) override;

		virtual VkBuffer GetBuffer() const;

	protected:
		VkDeviceMemory deviceMemory = nullptr;
		VkBuffer bufferObject = nullptr;
	};
}
