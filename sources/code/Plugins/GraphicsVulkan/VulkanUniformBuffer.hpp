#pragma once

#include <Common/Graphics/UniformBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class UniformBuffer : public Grindstone::GraphicsAPI::UniformBuffer {
	public:
		UniformBuffer(const CreateInfo& createInfo);
		virtual ~UniformBuffer();

		virtual void UpdateBuffer(void* content) override;
		virtual uint32_t GetSize() const override;
		virtual void Bind() override;

		virtual VkBuffer GetBuffer() const;
	private:
		VkDeviceMemory memory = nullptr;
		VkBuffer buffer = nullptr;
		uint32_t size = 0;
	};
}
