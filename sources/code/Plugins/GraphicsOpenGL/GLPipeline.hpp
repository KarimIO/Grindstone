#pragma once

#include <Common/Graphics/Pipeline.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class GLPipeline : public Pipeline {
			GLuint program_;
			GLuint primitive_type_;

			float width_, height_;
			int32_t scissor_x_, scissor_y_;
			uint32_t scissor_w_, scissor_h_;
			CullMode cull_mode_;

			GLuint createShaderModule(ShaderStageCreateInfo shaderStageCreateInfo);
		public:
			GLPipeline(Pipeline::CreateInfo& createInfo);
			void bind();
			GLuint getPrimitiveType();
			~GLPipeline();
		};
	}
}