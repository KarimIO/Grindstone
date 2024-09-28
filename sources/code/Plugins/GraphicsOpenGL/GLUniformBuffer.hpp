#pragma once

#include <Common/Graphics/UniformBuffer.hpp>

namespace Grindstone::GraphicsAPI {
	class GLUniformBuffer : public UniformBuffer {
	public:
		GLUniformBuffer(CreateInfo& createInfo);
		~GLUniformBuffer();

		virtual void UpdateBuffer(void *content) override;
		virtual uint32_t GetSize() override;
		virtual void Bind() override;
	private:
		GLuint uniformBufferObject = 0;
		GLuint bindingLocation = 0;
		uint32_t size = 0;
	};
}
