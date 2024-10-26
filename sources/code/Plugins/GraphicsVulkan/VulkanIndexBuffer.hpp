#pragma once

#include <Common/Graphics/IndexBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class IndexBuffer : public Grindstone::GraphicsAPI::IndexBuffer {
	public:
		IndexBuffer(const CreateInfo& createInfo);
		virtual ~IndexBuffer() override;
	public:
		VkBuffer GetBuffer() const;
		bool Is32Bit() const;
	private:
		VkBuffer buffer = nullptr;
		VkDeviceMemory memory = nullptr;
		bool is32Bit = false;
	};
}
