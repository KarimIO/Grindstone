#include <GL/gl3w.h>

#include <Common/Assert.hpp>

#include "GLFormats.hpp"

GLenum Grindstone::GraphicsAPI::TranslateMinFilterToOpenGL(bool hasMips, TextureFilter minFilter, TextureFilter mipFilter) {
	if (!hasMips) {
		return minFilter == TextureFilter::Linear
			? GL_LINEAR
			: GL_NEAREST;
	}

	if (mipFilter == TextureFilter::Linear) {
		return minFilter == TextureFilter::Linear
			? GL_NEAREST_MIPMAP_LINEAR
			: GL_NEAREST_MIPMAP_NEAREST;
	}

	return minFilter == TextureFilter::Linear
		? GL_LINEAR_MIPMAP_LINEAR
		: GL_LINEAR_MIPMAP_NEAREST;
}

GLenum Grindstone::GraphicsAPI::TranslateMagFilterToOpenGL(TextureFilter filter) {
	constexpr GLenum filters[] = {
		GL_NEAREST,
		GL_LINEAR
	};

	uint8_t index = static_cast<uint8_t>(filter);
	GS_ASSERT_ENGINE_WITH_MESSAGE(index < sizeof(filters) / sizeof(filters[0]), "TranslateMagFilterToOpenGL: Invalid filter value.");

	return filters[index];
}

GLenum Grindstone::GraphicsAPI::TranslateWrapToOpenGL(TextureWrapMode wrap) {
	constexpr GLenum wraps[] = {
		GL_REPEAT,
		GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_BORDER,
		GL_MIRRORED_REPEAT,
		GL_MIRROR_CLAMP_TO_EDGE
	};

	uint8_t index = static_cast<uint8_t>(wrap);
	GS_ASSERT_ENGINE_WITH_MESSAGE(index < sizeof(wraps) / sizeof(wraps[0]), "Invalid wrap value.");

	return wraps[index];
}

void Grindstone::GraphicsAPI::TranslateColorFormatToOpenGL(ColorFormat inFormat, bool& isCompressed, GLenum& format, GLint& internalFormat) {
	isCompressed = false;

	switch (inFormat) {
	case ColorFormat::RGB_DXT1:
		format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		isCompressed = true;
		break;
	case ColorFormat::RGBA_DXT1:
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		isCompressed = true;
		break;
	case ColorFormat::RGBA_DXT3:
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		isCompressed = true;
		break;
	case ColorFormat::RGBA_DXT5:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		isCompressed = true;
		break;
	case ColorFormat::R10G10B10A2:
		internalFormat = GL_RGB10_A2;
		format = GL_RGBA;
		break;
	case ColorFormat::R8:
		internalFormat = GL_R8;
		format = GL_RED;
		break;
	case ColorFormat::RG8:
		internalFormat = GL_RG8;
		format = GL_RG;
		break;
	case ColorFormat::RGB8:
		internalFormat = GL_RGB8;
		format = GL_RGB;
		break;
	case ColorFormat::RGBA8:
		internalFormat = GL_RGBA8;
		format = GL_RGBA;
		break;
	case ColorFormat::R16:
		internalFormat = GL_R16F;
		format = GL_RED;
		break;
	case ColorFormat::RG16:
		internalFormat = GL_RG16F;
		format = GL_RG;
		break;
	case ColorFormat::RGB16:
		internalFormat = GL_RGB16F;
		format = GL_RGB;
		break;
	case ColorFormat::RGBA16:
		internalFormat = GL_RGBA16F;
		format = GL_RGBA;
		break;
	case ColorFormat::RGB32:
		internalFormat = GL_RGB32F;
		format = GL_RGBA;
		break;
	case ColorFormat::RGBA32:
		internalFormat = GL_RGBA32F;
		format = GL_RGBA;
		break;
	default:
		GS_ASSERT_LOG("Invalid color format value.");
	};
}

void Grindstone::GraphicsAPI::TranslateDepthFormatToOpenGL(DepthFormat inFormat, GLenum& format, GLint& internalFormat) {
	switch (inFormat) {
	case DepthFormat::D16:
		internalFormat = GL_DEPTH_COMPONENT16;
		format = GL_DEPTH_COMPONENT;
		break;
	case DepthFormat::D24:
		internalFormat = GL_DEPTH_COMPONENT24;
		format = GL_DEPTH_COMPONENT;
		break;
	case DepthFormat::D32:
		internalFormat = GL_DEPTH_COMPONENT32F;
		format = GL_DEPTH_COMPONENT;
		break;
	case DepthFormat::D24_STENCIL_8:
		internalFormat = GL_DEPTH24_STENCIL8;
		format = GL_DEPTH_COMPONENT;
		break;
	case DepthFormat::D32_STENCIL_8:
		internalFormat = GL_DEPTH32F_STENCIL8;
		format = GL_DEPTH_COMPONENT;
		break;
	default:
		GS_ASSERT_LOG("Invalid depth format value.");
	};
}

GLenum Grindstone::GraphicsAPI::TranslateCullModeToOpenGL(CullMode cullMode) {
	GLenum culls[] = {
		GL_NONE,
		GL_FRONT,
		GL_BACK,
		GL_FRONT_AND_BACK
	};

	uint8_t index = static_cast<uint8_t>(cullMode);
	GS_ASSERT_ENGINE_WITH_MESSAGE(index < sizeof(culls) / sizeof(culls[0]), "Invalid cullMode value.");

	return culls[index];
}

GLuint Grindstone::GraphicsAPI::TranslatePolygonModeToOpenGL(PolygonFillMode mode) {
	constexpr GLenum modes[] = {
		GL_POINT,
		GL_LINE,
		GL_FILL
	};

	uint8_t index = static_cast<uint8_t>(mode);
	GS_ASSERT_ENGINE_WITH_MESSAGE(index < sizeof(modes) / sizeof(modes[0]), "Invalid PolygonFillMode value.");

	return modes[index];
}

GLuint Grindstone::GraphicsAPI::TranslateGeometryTypeToOpenGL(GeometryType geometryType) {
	constexpr GLenum types[] = {
		GL_POINTS,
		GL_LINES,
		GL_LINE_STRIP,
		GL_LINE_LOOP,
		GL_TRIANGLE_STRIP,
		GL_TRIANGLE_FAN,
		GL_TRIANGLES,
		GL_LINES_ADJACENCY,
		GL_TRIANGLES_ADJACENCY,
		GL_TRIANGLE_STRIP_ADJACENCY,
		GL_PATCHES
	};

	uint8_t index = static_cast<uint8_t>(geometryType);
	GS_ASSERT_ENGINE_WITH_MESSAGE(index < sizeof(types) / sizeof(types[0]), "Invalid geometryType value.");

	return types[index];
}

GLenum Grindstone::GraphicsAPI::TranslateBlendOpToOpenGL(BlendOperation op) {
	constexpr GLenum funcs[] = {
		GL_NONE,
		GL_FUNC_ADD,
		GL_FUNC_SUBTRACT,
		GL_FUNC_REVERSE_SUBTRACT,
		GL_MIN,
		GL_MAX,
		GL_NONE, // TODO: This should be GL_ZERO or something like that.
		GL_SRC_NV,
		GL_DST_NV,
		GL_SRC_OVER_NV,
		GL_DST_OVER_NV,
		GL_SRC_IN_NV,
		GL_DST_IN_NV,
		GL_SRC_OUT_NV,
		GL_DST_OUT_NV,
		GL_SRC_ATOP_NV,
		GL_DST_ATOP_NV,
		GL_XOR_NV,
		GL_MULTIPLY_KHR,
		GL_SCREEN_KHR,
		GL_OVERLAY_KHR,
		GL_DARKEN_KHR,
		GL_LIGHTEN_KHR,
		GL_COLORDODGE_KHR,
		GL_COLORBURN_KHR,
		GL_HARDLIGHT_KHR,
		GL_SOFTLIGHT_KHR,
		GL_DIFFERENCE_KHR,
		GL_EXCLUSION_KHR,
		GL_INVERT,
		GL_INVERT_RGB_NV,
		GL_LINEARDODGE_NV,
		GL_LINEARBURN_NV,
		GL_VIVIDLIGHT_NV,
		GL_LINEARLIGHT_NV,
		GL_PINLIGHT_NV,
		GL_HARDMIX_NV,
		GL_HSL_HUE_KHR,
		GL_HSL_SATURATION_KHR,
		GL_HSL_COLOR_KHR,
		GL_HSL_LUMINOSITY_KHR,
		GL_PLUS_NV,
		GL_PLUS_CLAMPED_NV,
		GL_PLUS_CLAMPED_ALPHA_NV,
		GL_PLUS_DARKER_NV,
		GL_MINUS_NV,
		GL_MINUS_CLAMPED_NV,
		GL_CONTRAST_NV,
		GL_INVERT_OVG_NV,
		GL_RED_NV,
		GL_GREEN_NV,
		GL_BLUE_NV
	};

	uint8_t index = static_cast<uint8_t>(op);
	GS_ASSERT_ENGINE_WITH_MESSAGE(index < sizeof(funcs) / sizeof(funcs[0]), "Invalid BlendOperation value.");

	return funcs[index];
}

GLenum Grindstone::GraphicsAPI::TranslateBlendFactorToOpenGL(BlendFactor factor) {
	constexpr GLenum factors[] = {
		GL_ZERO,
		GL_ONE,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_DST_COLOR,
		GL_ONE_MINUS_DST_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA,
		GL_CONSTANT_COLOR,
		GL_ONE_MINUS_CONSTANT_COLOR,
		GL_CONSTANT_ALPHA,
		GL_ONE_MINUS_CONSTANT_ALPHA,
		GL_SRC_ALPHA_SATURATE,
		GL_SRC1_COLOR,
		GL_ONE_MINUS_SRC1_COLOR,
		GL_SRC1_ALPHA,
		GL_ONE_MINUS_SRC1_ALPHA
	};

	uint8_t index = static_cast<uint8_t>(factor);
	GS_ASSERT_ENGINE_WITH_MESSAGE(index < sizeof(factors) / sizeof(factors[0]), "Invalid BlendFactor value.");

	return factors[index];
}

GLenum Grindstone::GraphicsAPI::TranslateCompareOpToOpenGL(CompareOperation op) {
	constexpr GLenum funcs[] = {
		GL_NEVER,
		GL_LESS,
		GL_EQUAL,
		GL_LEQUAL,
		GL_GREATER,
		GL_NOTEQUAL,
		GL_GEQUAL,
		GL_ALWAYS
	};

	uint8_t index = static_cast<uint8_t>(op);
	GS_ASSERT_ENGINE_WITH_MESSAGE(index < sizeof(funcs) / sizeof(funcs[0]), "Invalid CompareOperation value.");

	return funcs[index];
}

GLenum Grindstone::GraphicsAPI::TranslateVertexFormatToOpenGL(VertexFormat format) {
	switch (format) {
		case VertexFormat::Float:
		case VertexFormat::Float2:
		case VertexFormat::Float3:
		case VertexFormat::Float4:
		case VertexFormat::Mat3:
		case VertexFormat::Mat4:	return GL_FLOAT;

		case VertexFormat::Int:
		case VertexFormat::Int2:
		case VertexFormat::Int3:
		case VertexFormat::Int4:	return GL_INT;

		case VertexFormat::UInt:
		case VertexFormat::UInt2:
		case VertexFormat::UInt3:
		case VertexFormat::UInt4:	return GL_UNSIGNED_INT;
		case VertexFormat::Bool:	return GL_BOOL;
	}

	GS_ASSERT_LOG("Invalid VertexFormat!");

	return 0;
}
