#pragma once

#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/VertexBuffer.hpp>

namespace Grindstone::GraphicsAPI {
	GLenum TranslateVertexFormatToOpenGL(VertexFormat format);
	GLenum TranslateMinFilterToOpenGL(bool hasMips, TextureFilter minFilter, TextureFilter mipFilter);
	GLenum TranslateMagFilterToOpenGL(TextureFilter);
	GLenum TranslateWrapToOpenGL(TextureWrapMode);
	void TranslateColorFormatToOpenGL(ColorFormat inFormat, bool& isCompressed, GLenum& format, GLint& internalFormat);
	void TranslateDepthFormatToOpenGL(DepthFormat inFormat, GLenum& format, GLint& internalFormat);
	GLenum TranslateCullModeToOpenGL(CullMode cullMode);
	GLenum TranslatePolygonModeToOpenGL(PolygonFillMode mode);
	GLenum TranslateGeometryTypeToOpenGL(GeometryType geometryType);

	GLenum TranslateBlendOpToOpenGL(BlendOperation op);
	GLenum TranslateBlendFactorToOpenGL(BlendFactor factor);
	GLenum TranslateCompareOpToOpenGL(CompareOperation op);
}
