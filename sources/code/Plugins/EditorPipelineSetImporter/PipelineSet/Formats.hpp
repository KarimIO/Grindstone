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

	#define SHADER_STAGE_TYPES \
		GSExpandEntry(Vertex, 1 << 0),\
		GSExpandEntry(TesselationEvaluation, 1 << 1),\
		GSExpandEntry(TesselationControl, 1 << 2),\
		GSExpandEntry(Geometry, 1 << 3),\
		GSExpandEntry(Fragment, 1 << 4),\
		GSExpandEntry(Task, 1 << 5),\
		GSExpandEntry(Mesh, 1 << 6),\
		GSExpandEntry(Compute, 1 << 7)

	enum class ShaderStage : uint8_t {
		#define GSExpandEntry(key, bit) key
		SHADER_STAGE_TYPES,
		#undef GSExpandEntry
		GraphicsCount = Compute,
		Count
	};

	constexpr uint8_t numShaderGraphicStage = static_cast<uint8_t>(ShaderStage::GraphicsCount);
	constexpr uint8_t numShaderTotalStage = static_cast<uint8_t>(ShaderStage::Count);

	enum class ShaderStageBit : uint8_t {
		None = 0,
#define GSExpandEntry(key, bit) key = bit
		SHADER_STAGE_TYPES,
#undef GSExpandEntry
		AllGraphics = Vertex | TesselationEvaluation | TesselationControl | Geometry | Fragment | Task | Mesh,
		All = AllGraphics | Compute
	};

	constexpr const char* shaderStageNames[] = {
		#define GSExpandEntry(key, bit) #key
		SHADER_STAGE_TYPES
		#undef GSExpandEntry
	};

	inline const char* GetShaderStageName(Grindstone::GraphicsAPI::ShaderStage stage) {
		uint8_t index = static_cast<uint8_t>(stage);
		if (index >= static_cast<uint8_t>(ShaderStage::Count)) {
			return "Invalid";
		}

		return shaderStageNames[index];
	}

	enum class BindingType {
		UniformBuffer,
		Texture,
		RenderTexture,
		DepthTexture,
		RenderTextureStorageImage
	};

#define BLEND_OPERATIONS_LIST \
	GSExpandEntry(None),\
	GSExpandEntry(Add),\
	GSExpandEntry(Subtract),\
	GSExpandEntry(ReverseSubtract),\
	GSExpandEntry(Minimum),\
	GSExpandEntry(Maximum),\
	GSExpandEntry(Zero),\
	GSExpandEntry(Source),\
	GSExpandEntry(Destination),\
	GSExpandEntry(SourceOver),\
	GSExpandEntry(DestinationOver),\
	GSExpandEntry(SourceIn),\
	GSExpandEntry(DestinationIn),\
	GSExpandEntry(SourceOut),\
	GSExpandEntry(DestinationOut),\
	GSExpandEntry(SourceAtop),\
	GSExpandEntry(DestinationAtop),\
	GSExpandEntry(XOR),\
	GSExpandEntry(Multiply),\
	GSExpandEntry(Screen),\
	GSExpandEntry(Overlay),\
	GSExpandEntry(Darken),\
	GSExpandEntry(Lighten),\
	GSExpandEntry(ColorDodge),\
	GSExpandEntry(ColorBurn),\
	GSExpandEntry(HardLight),\
	GSExpandEntry(SoftLight),\
	GSExpandEntry(Difference),\
	GSExpandEntry(Exclusion),\
	GSExpandEntry(Invert),\
	GSExpandEntry(InvertRGB),\
	GSExpandEntry(LinearDodge),\
	GSExpandEntry(LinearBurn),\
	GSExpandEntry(VividLight),\
	GSExpandEntry(LinearLight),\
	GSExpandEntry(PinLight),\
	GSExpandEntry(HardMix),\
	GSExpandEntry(HSLHue),\
	GSExpandEntry(HSLSaturation),\
	GSExpandEntry(HSLColor),\
	GSExpandEntry(HSLLuminosity),\
	GSExpandEntry(Plus),\
	GSExpandEntry(PlusClamped),\
	GSExpandEntry(PlusClampedAlpha),\
	GSExpandEntry(PlusDark),\
	GSExpandEntry(Minus),\
	GSExpandEntry(MinusClamped),\
	GSExpandEntry(Contrast),\
	GSExpandEntry(InvertOVG),\
	GSExpandEntry(Red),\
	GSExpandEntry(Green),\
	GSExpandEntry(Blue)

	enum class BlendOperation : uint8_t {
#define GSExpandEntry(key) key
		BLEND_OPERATIONS_LIST,
#undef GSExpandEntry
		Count
	};

	constexpr const char* blendOperationNames[] = {
		#define GSExpandEntry(key) #key
		BLEND_OPERATIONS_LIST
		#undef GSExpandEntry
	};

	inline const char* GetBlendOperationName(Grindstone::GraphicsAPI::BlendOperation op) {
		uint8_t index = static_cast<uint8_t>(op);
		if (index >= static_cast<uint8_t>(BlendOperation::Count)) {
			return "Invalid";
		}

		return blendOperationNames[index];
	}


#define BLEND_FACTORS_LIST \
	GSExpandEntry(Zero),\
	GSExpandEntry(One),\
	GSExpandEntry(SrcColor),\
	GSExpandEntry(OneMinusSrcColor),\
	GSExpandEntry(DstColor),\
	GSExpandEntry(OneMinusDstColor),\
	GSExpandEntry(SrcAlpha),\
	GSExpandEntry(OneMinusSrcAlpha),\
	GSExpandEntry(DstAlpha),\
	GSExpandEntry(OneMinusDstAlpha),\
	GSExpandEntry(ConstantColor),\
	GSExpandEntry(OneMinusConstantColor),\
	GSExpandEntry(ConstantAlpha),\
	GSExpandEntry(OneMinusConstantAlpha),\
	GSExpandEntry(SrcAlphaSaturate),\
	GSExpandEntry(Src1Color),\
	GSExpandEntry(OneMinusSrc1Color),\
	GSExpandEntry(Src1Alpha),\
	GSExpandEntry(OneMinusSrc1Alpha)

	enum class BlendFactor : uint8_t {
		#define GSExpandEntry(key) key
		BLEND_FACTORS_LIST,
		#undef GSExpandEntry
		Count
	};

	constexpr const char* blendFactorNames[] = {
		#define GSExpandEntry(key) #key
		BLEND_FACTORS_LIST
		#undef GSExpandEntry
	};

	inline const char* GetBlendFactorName(Grindstone::GraphicsAPI::BlendFactor factor) {
		uint8_t index = static_cast<uint8_t>(factor);
		if (index >= static_cast<uint8_t>(BlendFactor::Count)) {
			return "Invalid";
		}

		return blendFactorNames[index];
	}

#define GEOMETRY_TYPES_LIST \
		GSExpandEntry(Points),\
		GSExpandEntry(Lines),\
		GSExpandEntry(LineStrips),\
		GSExpandEntry(LineLoops),\
		GSExpandEntry(TriangleStrips),\
		GSExpandEntry(TriangleFans),\
		GSExpandEntry(Triangles),\
		GSExpandEntry(LinesAdjacency),\
		GSExpandEntry(TrianglesAdjacency),\
		GSExpandEntry(TriangleStripsAdjacency),\
		GSExpandEntry(Patches)

	enum class GeometryType : uint8_t {
		#define GSExpandEntry(key) key
		GEOMETRY_TYPES_LIST,
		#undef GSExpandEntry
		Count
	};

	constexpr const char* geometryTypeNames[] = {
		#define GSExpandEntry(key) #key
		GEOMETRY_TYPES_LIST
		#undef GSExpandEntry
	};

	inline const char* GetGeometryTypeName(Grindstone::GraphicsAPI::GeometryType stage) {
		uint8_t index = static_cast<uint8_t>(stage);
		if (index >= static_cast<uint8_t>(GeometryType::Count)) {
			return "Invalid";
		}

		return geometryTypeNames[index];
	}

	enum class PolygonFillMode : uint8_t {
		Point,
		Line,
		Fill
	};

	constexpr const char* polygonFillModeNames[] = {
		"Point",
		"Line",
		"Fill"
	};

	inline const char* GetPolygonFillModeName(Grindstone::GraphicsAPI::PolygonFillMode mode) {
		uint8_t index = static_cast<uint8_t>(mode);
		if (index > static_cast<uint8_t>(PolygonFillMode::Fill)) {
			return "Invalid";
		}

		return polygonFillModeNames[index];
	}

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

	constexpr const char* compareOperationNames[] = {
		#define GSExpandEntry(key, bit) #key
		SHADER_STAGE_TYPES
		#undef GSExpandEntry
	};

	inline const char* GetCompareOperationName(Grindstone::GraphicsAPI::CompareOperation op) {
		uint8_t index = static_cast<uint8_t>(op);
		if (index > static_cast<uint8_t>(CompareOperation::Always)) {
			return "Invalid";
		}

		return compareOperationNames[index];
	}

	enum class ColorMask : uint8_t {
		None = 0,
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

	constexpr const char* colorMaskNames[] = {
		"None",
		"R",
		"G",
		"RG",
		"B",
		"RB",
		"GB",
		"RGB",
		"A",
		"RA",
		"GA",
		"RGA",
		"BA",
		"RBA",
		"GBA",
		"RGBA"
	};

	inline const char* GetColorMaskName(Grindstone::GraphicsAPI::ColorMask colorMask) {
		uint8_t index = static_cast<uint8_t>(colorMask);
		if (index > static_cast<uint8_t>(ColorMask::RGBA)) {
			return "Invalid";
		}

		return colorMaskNames[index];
	}

	enum class CullMode : uint8_t {
		None = 0,
		Front,
		Back,
		Both
	};

	constexpr const char* cullModeNames[] = {
		"None",
		"Front",
		"Back",
		"Both"
	};

	inline const char* GetCullModeName(Grindstone::GraphicsAPI::CullMode cullMode) {
		uint8_t index = static_cast<uint8_t>(cullMode);
		if (index > static_cast<uint8_t>(CullMode::Both)) {
			return "Invalid";
		}

		return cullModeNames[index];
	}
}

inline Grindstone::GraphicsAPI::ShaderStageBit operator~(const Grindstone::GraphicsAPI::ShaderStageBit stages) {
	using ShaderStageBitType = std::underlying_type_t<Grindstone::GraphicsAPI::ShaderStageBit>;
	return static_cast<Grindstone::GraphicsAPI::ShaderStageBit>(~static_cast<ShaderStageBitType>(stages));
}

inline Grindstone::GraphicsAPI::ShaderStageBit operator|(const Grindstone::GraphicsAPI::ShaderStageBit a, const Grindstone::GraphicsAPI::ShaderStageBit b) {
	using ShaderStageBitType = std::underlying_type_t<Grindstone::GraphicsAPI::ShaderStageBit>;
	return static_cast<Grindstone::GraphicsAPI::ShaderStageBit>(static_cast<ShaderStageBitType>(a) | static_cast<ShaderStageBitType>(a));
}

inline Grindstone::GraphicsAPI::ShaderStageBit operator&(const Grindstone::GraphicsAPI::ShaderStageBit a, const Grindstone::GraphicsAPI::ShaderStageBit b) {
	using ShaderStageBitType = std::underlying_type_t<Grindstone::GraphicsAPI::ShaderStageBit>;
	return static_cast<Grindstone::GraphicsAPI::ShaderStageBit>(static_cast<ShaderStageBitType>(a) & static_cast<ShaderStageBitType>(a));
}

inline Grindstone::GraphicsAPI::ShaderStageBit operator^(const Grindstone::GraphicsAPI::ShaderStageBit a, const Grindstone::GraphicsAPI::ShaderStageBit b) {
	using ShaderStageBitType = std::underlying_type_t<Grindstone::GraphicsAPI::ShaderStageBit>;
	return static_cast<Grindstone::GraphicsAPI::ShaderStageBit>(static_cast<ShaderStageBitType>(a) ^ static_cast<ShaderStageBitType>(a));
}

inline Grindstone::GraphicsAPI::ShaderStageBit& operator|=(Grindstone::GraphicsAPI::ShaderStageBit& a, const Grindstone::GraphicsAPI::ShaderStageBit b) {
	a = a | b;
	return a;
}

inline Grindstone::GraphicsAPI::ShaderStageBit& operator&=(Grindstone::GraphicsAPI::ShaderStageBit& a, const Grindstone::GraphicsAPI::ShaderStageBit b) {
	a = a & b;
	return a;
}

inline Grindstone::GraphicsAPI::ShaderStageBit& operator^=(Grindstone::GraphicsAPI::ShaderStageBit& a, const Grindstone::GraphicsAPI::ShaderStageBit b) {
	a = a ^ b;
	return a;
}

inline Grindstone::GraphicsAPI::ColorMask operator~(const Grindstone::GraphicsAPI::ColorMask stages) {
	using ColorMaskType = std::underlying_type_t<Grindstone::GraphicsAPI::ColorMask>;
	return static_cast<Grindstone::GraphicsAPI::ColorMask>(~static_cast<ColorMaskType>(stages));
}

inline Grindstone::GraphicsAPI::ColorMask operator|(const Grindstone::GraphicsAPI::ColorMask a, const Grindstone::GraphicsAPI::ColorMask b) {
	using ColorMaskType = std::underlying_type_t<Grindstone::GraphicsAPI::ColorMask>;
	return static_cast<Grindstone::GraphicsAPI::ColorMask>(static_cast<ColorMaskType>(a) | static_cast<ColorMaskType>(a));
}

inline Grindstone::GraphicsAPI::ColorMask operator&(const Grindstone::GraphicsAPI::ColorMask a, const Grindstone::GraphicsAPI::ColorMask b) {
	using ColorMaskType = std::underlying_type_t<Grindstone::GraphicsAPI::ColorMask>;
	return static_cast<Grindstone::GraphicsAPI::ColorMask>(static_cast<ColorMaskType>(a) & static_cast<ColorMaskType>(a));
}

inline Grindstone::GraphicsAPI::ColorMask operator^(const Grindstone::GraphicsAPI::ColorMask a, const Grindstone::GraphicsAPI::ColorMask b) {
	using ColorMaskType = std::underlying_type_t<Grindstone::GraphicsAPI::ColorMask>;
	return static_cast<Grindstone::GraphicsAPI::ColorMask>(static_cast<ColorMaskType>(a) ^ static_cast<ColorMaskType>(a));
}

inline Grindstone::GraphicsAPI::ColorMask& operator|=(Grindstone::GraphicsAPI::ColorMask& a, const Grindstone::GraphicsAPI::ColorMask b) {
	a = a | b;
	return a;
}

inline Grindstone::GraphicsAPI::ColorMask& operator&=(Grindstone::GraphicsAPI::ColorMask& a, const Grindstone::GraphicsAPI::ColorMask b) {
	a = a & b;
	return a;
}

inline Grindstone::GraphicsAPI::ColorMask& operator^=(Grindstone::GraphicsAPI::ColorMask& a, const Grindstone::GraphicsAPI::ColorMask b) {
	a = a ^ b;
	return a;
}
