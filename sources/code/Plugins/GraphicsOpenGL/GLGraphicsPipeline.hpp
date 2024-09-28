#pragma once

#include <Common/Graphics/GraphicsPipeline.hpp>

namespace Grindstone::GraphicsAPI {
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

		GLsizei width, height;
		GLint scissorX, scissorY;
		GLsizei scissorWidth, scissorHeight;

		GLboolean colorMaskRed = GL_TRUE;
		GLboolean colorMaskBlue = GL_TRUE;
		GLboolean colorMaskGreen = GL_TRUE;
		GLboolean colorMaskAlpha = GL_TRUE;
		GLuint primitiveType;
		GLenum polygonFillMode;
		GLenum depthCompareOp;
		bool isDepthTestEnabled = true;
		GLboolean isDepthWriteEnabled = GL_TRUE;
		bool isStencilEnabled = false;
		bool isDepthBiasEnabled = false;
		bool isDepthClampEnabled = false;

		float depthBiasConstantFactor = 1.25f;
		float depthBiasSlopeFactor = 1.75f;
		float depthBiasClamp = 0.0f;

		GLenum cullMode;
		GLenum blendColorOp;
		GLenum blendColorSrc;
		GLenum blendColorDst;
		GLenum blendAlphaOp;
		GLenum blendAlphaSrc;
		GLenum blendAlphaDst;
	};
}
