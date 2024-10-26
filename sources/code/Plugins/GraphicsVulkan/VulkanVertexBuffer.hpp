#pragma once

#include <Common/Graphics/VertexBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class VertexBuffer : public Grindstone::GraphicsAPI::VertexBuffer {
	public:
		VertexBuffer(const CreateInfo& createInfo);
		virtual ~VertexBuffer() override;
	public:
		VkBuffer GetBuffer() const;
	private:
		VkBuffer buffer = nullptr;
		VkDeviceMemory memory = nullptr;
	};
}
