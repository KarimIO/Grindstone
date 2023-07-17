#pragma once

#include <Common/Graphics/GraphicsPipeline.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class GLGraphicsPipeline : public GraphicsPipeline {
		public:
			GLGraphicsPipeline(GraphicsPipeline::CreateInfo& createInfo);
			virtual void Recreate(GraphicsPipeline::CreateInfo& createInfo) override;
			void Bind();
			GLuint GetPrimitiveType();
			~GLGraphicsPipeline();
		private:
			void CreatePipeline(GraphicsPipeline::CreateInfo& createInfo);
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
