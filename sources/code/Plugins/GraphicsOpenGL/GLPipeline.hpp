#pragma once

#include <Common/Graphics/Pipeline.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class GLPipeline : public Pipeline {
		public:
			GLPipeline(CreateInfo& createInfo);
			virtual void Recreate(CreateInfo& createInfo) override;
			void Bind();
			GLuint GetPrimitiveType();
			~GLPipeline();
		private:
			void CreatePipeline(CreateInfo& createInfo);
			GLuint CreateShaderModule(ShaderStageCreateInfo shaderStageCreateInfo);

			GLuint program;
			GLuint primitiveType;

			float width, height;
			int32_t scissorX, scissorY;
			uint32_t scissorWidth, scissorHeight;
			CullMode cullMode;
		};
	}
}