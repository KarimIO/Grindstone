#pragma once

#include <stdint.h>

namespace Grindstone::GraphicsAPI {
	enum class ClearMode : uint8_t {
		Color = 1,
		Depth = 2,
		ColorAndDepth = 3,
		Stencil = 4,
		All = 7
	};

	enum class ColorFormat : uint8_t {
		Invalid = 0,

		R8,
		RG8,
		RGB8,
		RGBA8,

		R10G10B10A2,

		R16,
		R32,
		RG16,
		RG32,
		RGB16,
		RGBA16,
		RGB32,
		RGBA32,

		RGB_DXT1,
		RGBA_DXT1,
		RGBA_DXT3,
		RGBA_DXT5,

		SRGB_DXT1,
		SRGB_ALPHA_DXT1,
		SRGB_ALPHA_DXT3,
		SRGB_ALPHA_DXT5,

		BC4,
		BC6H
	};

	enum class DepthFormat : uint8_t {
		None = 0,
		D16,
		D24,
		D32,
		//D16_STENCIL_8,
		D24_STENCIL_8,
		D32_STENCIL_8
		//FORMAT_STENCIL_8
	};

	enum class ShaderStage : uint8_t {
		Vertex = 0,
		TesselationEvaluation,
		TesselationControl,
		Geometry,
		Fragment,
		NumGraphics,
		Compute = NumGraphics,
		All
	};

	enum class ShaderStageBit : uint8_t {
		Vertex = 0x1,
		TesselationEvaluation = 0x2,
		TesselationControl = 0x4,
		Geometry = 0x8,
		Fragment = 0x10,
		AllGraphics = 0x1F,
		Compute = 0x20,
		All = 0x3F
	};

	ShaderStageBit operator |(ShaderStageBit a, ShaderStageBit b);

	enum class BindingType {
		UniformBuffer,
		Texture,
		RenderTexture,
		DepthTexture,
		RenderTextureStorageImage
	};

	const uint8_t numShaderGraphicStage = 5;
	const uint8_t numShaderTotalStage = 6;

	enum class BlendOperation : uint8_t {
		None = 0,
		Add,
		Subtract,
		ReverseSubtract,
		Minimum,
		Maximum,
		Zero,
		Source,
		Destination,
		SourceOver,
		DestinationOver,
		SourceIn,
		DestinationIn,
		SourceOut,
		DestinationOut,
		SourceAtop,
		DestinationAtop,
		XOR,
		Multiply,
		Screen,
		Overlay,
		Darken,
		Lighten,
		ColorDodge,
		ColorBurn,
		HardLight,
		SoftLight,
		Difference,
		Exclusion,
		Invert,
		InvertRGB,
		LinearDodge,
		LinearBurn,
		VividLight,
		LinearLight,
		PinLight,
		HardMix,
		HSLHue,
		HSLSaturation,
		HSLColor,
		HSLLuminosity,
		Plus,
		PlusClamped,
		PlusClampedAlpha,
		PlusDark,
		Minus,
		MinusClamped,
		Contrast,
		InvertOVG,
		Red,
		Green,
		Blue,
	};

	enum class BlendFactor : uint8_t {
		Zero = 0,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha,
		ConstantColor,
		OneMinusConstantColor,
		ConstantAlpha,
		OneMinusConstantAlpha,
		SrcAlphaSaturate,
		Src1Color,
		OneMinusSrc1Color,
		Src1Alpha,
		OneMinusSrc1Alpha
	};

	struct BlendData {
		BlendOperation colorOperation = BlendOperation::None;
		BlendFactor colorFactorSrc = BlendFactor::One;
		BlendFactor colorFactorDst = BlendFactor::One;

		BlendOperation alphaOperation = BlendOperation::None;
		BlendFactor alphaFactorSrc = BlendFactor::One;
		BlendFactor alphaFactorDst = BlendFactor::One;

		static BlendData NoBlending() {
			return BlendData{
				BlendOperation::None,
				BlendFactor::One,
				BlendFactor::One,

				BlendOperation::None,
				BlendFactor::One,
				BlendFactor::One
			};
		};

		static BlendData Additive() {
			return BlendData{
				BlendOperation::Add,
				BlendFactor::One,
				BlendFactor::One,

				BlendOperation::Add,
				BlendFactor::One,
				BlendFactor::One
			};
		};

		static BlendData AdditiveAlpha() {
			return BlendData{
				BlendOperation::Add,
				BlendFactor::SrcAlpha,
				BlendFactor::OneMinusSrcAlpha,

				BlendOperation::Add,
				BlendFactor::One,
				BlendFactor::OneMinusSrcAlpha
			};
		};
	};

	enum class GeometryType : uint8_t {
		Points,
		Lines,
		LineStrips,
		LineLoops,
		TriangleStrips,
		TriangleFans,
		Triangles,
		LinesAdjacency,
		TrianglesAdjacency,
		TriangleStripsAdjacency,
		Patches
	};

	enum class PolygonFillMode : uint8_t {
		Point,
		Line,
		Fill
	};

	enum class CompareOperation : uint8_t {
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always
	};

	enum class TextureWrapMode : uint8_t {
		Repeat = 0,
		ClampToEdge,
		ClampToBorder,
		MirroredRepeat,
		MirroredClampToEdge
	};

	enum class TextureFilter : uint8_t {
		Nearest = 0,
		Linear
	};

	enum class ColorMask : uint8_t {
		Red = 0x1,
		Green = 0x2,
		Blue = 0x4,
		Alpha = 0x8,

		RG = Red | Green,
		RB = Red | Blue,
		RA = Red | Alpha,
		GB = Green | Blue,
		GA = Green | Alpha,
		BA = Blue | Alpha,

		RGB = Red | Green | Blue,
		RGA = Red | Green | Alpha,
		RBA = Red | Blue | Alpha,
		GBA = Green | Blue | Alpha,

		RGBA = Red | Green | Blue | Alpha
	};

	ColorMask operator~(const ColorMask& f);
	ColorMask operator|(const ColorMask& a, const ColorMask& b);
	ColorMask operator&(const ColorMask& a, const ColorMask& b);

	enum class CullMode : uint8_t {
		None = 0,
		Front,
		Back,
		Both
	};
}
