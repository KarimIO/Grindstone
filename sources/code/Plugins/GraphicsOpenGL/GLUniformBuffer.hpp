#pragma once

#include <Common/Graphics/UniformBuffer.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class UniformBuffer : public Grindstone::GraphicsAPI::UniformBuffer {
	public:
		UniformBuffer(const CreateInfo& createInfo);
		~UniformBuffer();

		virtual void UpdateBuffer(void *content) override;
		virtual uint32_t GetSize() const override;
		virtual void Bind() override;
	private:
		GLuint uniformBufferObject = 0;
		GLuint bindingLocation = 0;
		uint32_t size = 0;
	};
}
