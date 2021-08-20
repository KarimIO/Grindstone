#pragma once

#include <Common/Graphics/Pipeline.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class GLPipeline : public Pipeline {
			GLuint program;
			GLuint primitiveType;

			float width, height;
			int32_t scissorX, scissorY;
			uint32_t scissorWidth, scissorHeight;
			CullMode cullMode;

			GLuint CreateShaderModule(ShaderStageCreateInfo shaderStageCreateInfo);
		public:
			GLPipeline(Pipeline::CreateInfo& createInfo);
			void Bind();
			GLuint GetPrimitiveType();
			~GLPipeline();
		};
	}
}