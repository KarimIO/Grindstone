#pragma once

#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/Buffer.hpp>

namespace Grindstone::GraphicsAPI {

	struct OpenGLFormats {
		GLenum internalFormat;
		GLenum format;
		GLenum type;
	};

	OpenGLFormats TranslateFormatToOpenGL(Format format);
	GLenum TranslateMinFilterToOpenGL(bool hasMips, TextureFilter minFilter, TextureFilter mipFilter);
	GLenum TranslateMagFilterToOpenGL(TextureFilter);
	GLenum TranslateWrapToOpenGL(TextureWrapMode);
	GLenum TranslateCullModeToOpenGL(CullMode cullMode);
	GLenum TranslatePolygonModeToOpenGL(PolygonFillMode mode);
	GLenum TranslateGeometryTypeToOpenGL(GeometryType geometryType);

	GLenum TranslateBlendOpToOpenGL(BlendOperation op);
	GLenum TranslateBlendFactorToOpenGL(BlendFactor factor);
	GLenum TranslateCompareOpToOpenGL(CompareOperation op);
}
