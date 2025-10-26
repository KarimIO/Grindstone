#pragma once

#include <vector>
#include <utility>
#include <stdint.h>

namespace Grindstone::GraphicsAPI {
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

	enum class ClearMode : uint8_t {
		Color = 1,
		Depth = 2,
		ColorAndDepth = 3,
		Stencil = 4,
		All = 7
	};

	enum class ImageLayout : uint8_t {
		Undefined,				// For newly created or discarded images
		General,				// For compute or unordered read/write
		ColorAttachment,		// Render target output
		DepthWrite,				// Depth write access
		DepthRead,				// Depth sampled (read-only)
		ShaderRead,				// Sampled image (color or depth)
		TransferSrc,			// Used as a copy/blit source
		TransferDst,			// Used as a copy/blit destination
		Present,				// Presented to screen
	};

	enum class MemoryUsage {
		GPUOnly,				// Device-local VRAM, inaccessible to CPU.
		CPUToGPU,				// Host-visible upload buffer (CPU writes, GPU reads)
		GPUToCPU,				// Host-visible readback buffer (GPU writes, CPU reads)
		CPUOnly,				// Host-visible, used as staging/upload/readback
		Transient,				// Transient GPU memory (tile memory / renderpass-only)
	};

	enum class ImageDimension {
		Invalid,
		Dimension1D,
		Dimension2D,
		Dimension3D
	};

	enum class ImageAspectBits : uint16_t {
		None = 0,
		Color = 0x00000001,
		Depth = 0x00000002,
		Stencil = 0x00000004,
		Metadata = 0x00000008,
		Plane0 = 0x00000010,
		Plane1 = 0x00000020,
		Plane2 = 0x00000040,
		MemoryPlane0 = 0x00000080,
		MemoryPlane1 = 0x00000100,
		MemoryPlane2 = 0x00000200,
		MemoryPlane3 = 0x00000400,
	};

	inline ImageAspectBits operator|(ImageAspectBits a, ImageAspectBits b) {
		return static_cast<ImageAspectBits>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline ImageAspectBits& operator|=(ImageAspectBits& a, ImageAspectBits b) {
		a = a | b;
		return a;
	}

	enum class PipelineStageBit : uint32_t {
		None = 0,
		TopOfPipe = 0x00000001,
		DrawIndirect = 0x00000002,
		VertexInput = 0x00000004,
		VertexShader = 0x00000008,
		TesslationControlShader = 0x00000010,
		TesselationEvaluationShader = 0x00000020,
		GeometryShader = 0x00000040,
		FragmentShader = 0x00000080,
		EarlyFragmentTests = 0x00000100,
		LateFragmentTests = 0x00000200,
		ColorAttachmentOutput = 0x00000400,
		ComputeShader = 0x00000800,
		Transfer = 0x00001000,
		BottomOfPipe = 0x00002000,
		Host = 0x00004000,
		AllGraphics = 0x00008000,
		AllCommands = 0x00010000,
		TransformFeedback = 0x01000000,
		ConditionalRendering = 0x00040000,
		AccelerationStructureBuild = 0x02000000,
		RayTracingShader = 0x00200000,
		FragmentDensityProcess = 0x00800000,
		FragmentShaderRateAttachment = 0x00400000,
		CommandPreprocess = 0x00020000,
		TaskShader = 0x00080000,
		MeshShader = 0x00100000,
	};

	inline PipelineStageBit operator|(PipelineStageBit a, PipelineStageBit b) {
		return static_cast<PipelineStageBit>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline PipelineStageBit& operator|=(PipelineStageBit& a, PipelineStageBit b) {
		a = a | b;
		return a;
	}

	enum class Format {
		Invalid,
		R4G4_UNORM_PACK8,
		R4G4B4A4_UNORM_PACK16,
		B4G4R4A4_UNORM_PACK16,
		R5G6B5_UNORM_PACK16,
		B5G6R5_UNORM_PACK16,
		R5G5B5A1_UNORM_PACK16,
		B5G5R5A1_UNORM_PACK16,
		A1R5G5B5_UNORM_PACK16,
		R8_UNORM,
		R8_SNORM,
		R8_USCALED,
		R8_SSCALED,
		R8_UINT,
		R8_SINT,
		R8_SRGB,
		R8G8_UNORM,
		R8G8_SNORM,
		R8G8_USCALED,
		R8G8_SSCALED,
		R8G8_UINT,
		R8G8_SINT,
		R8G8_SRGB,
		R8G8B8_UNORM,
		R8G8B8_SNORM,
		R8G8B8_USCALED,
		R8G8B8_SSCALED,
		R8G8B8_UINT,
		R8G8B8_SINT,
		R8G8B8_SRGB,
		B8G8R8_UNORM,
		B8G8R8_SNORM,
		B8G8R8_USCALED,
		B8G8R8_SSCALED,
		B8G8R8_UINT,
		B8G8R8_SINT,
		B8G8R8_SRGB,
		R8G8B8A8_UNORM,
		R8G8B8A8_SNORM,
		R8G8B8A8_USCALED,
		R8G8B8A8_SSCALED,
		R8G8B8A8_UINT,
		R8G8B8A8_SINT,
		R8G8B8A8_SRGB,
		B8G8R8A8_UNORM,
		B8G8R8A8_SNORM,
		B8G8R8A8_USCALED,
		B8G8R8A8_SSCALED,
		B8G8R8A8_UINT,
		B8G8R8A8_SINT,
		B8G8R8A8_SRGB,
		A8B8G8R8_UNORM_PACK32,
		A8B8G8R8_SNORM_PACK32,
		A8B8G8R8_USCALED_PACK32,
		A8B8G8R8_SSCALED_PACK32,
		A8B8G8R8_UINT_PACK32,
		A8B8G8R8_SINT_PACK32,
		A8B8G8R8_SRGB_PACK32,
		A2R10G10B10_UNORM_PACK32,
		A2R10G10B10_SNORM_PACK32,
		A2R10G10B10_USCALED_PACK32,
		A2R10G10B10_SSCALED_PACK32,
		A2R10G10B10_UINT_PACK32,
		A2R10G10B10_SINT_PACK32,
		A2B10G10R10_UNORM_PACK32,
		A2B10G10R10_SNORM_PACK32,
		A2B10G10R10_USCALED_PACK32,
		A2B10G10R10_SSCALED_PACK32,
		A2B10G10R10_UINT_PACK32,
		A2B10G10R10_SINT_PACK32,
		R16_UNORM,
		R16_SNORM,
		R16_USCALED,
		R16_SSCALED,
		R16_UINT,
		R16_SINT,
		R16_SFLOAT,
		R16G16_UNORM,
		R16G16_SNORM,
		R16G16_USCALED,
		R16G16_SSCALED,
		R16G16_UINT,
		R16G16_SINT,
		R16G16_SFLOAT,
		R16G16B16_UNORM,
		R16G16B16_SNORM,
		R16G16B16_USCALED,
		R16G16B16_SSCALED,
		R16G16B16_UINT,
		R16G16B16_SINT,
		R16G16B16_SFLOAT,
		R16G16B16A16_UNORM,
		R16G16B16A16_SNORM,
		R16G16B16A16_USCALED,
		R16G16B16A16_SSCALED,
		R16G16B16A16_UINT,
		R16G16B16A16_SINT,
		R16G16B16A16_SFLOAT,
		R32_UINT,
		R32_SINT,
		R32_SFLOAT,
		R32G32_UINT,
		R32G32_SINT,
		R32G32_SFLOAT,
		R32G32B32_UINT,
		R32G32B32_SINT,
		R32G32B32_SFLOAT,
		R32G32B32A32_UINT,
		R32G32B32A32_SINT,
		R32G32B32A32_SFLOAT,
		R64_UINT,
		R64_SINT,
		R64_SFLOAT,
		R64G64_UINT,
		R64G64_SINT,
		R64G64_SFLOAT,
		R64G64B64_UINT,
		R64G64B64_SINT,
		R64G64B64_SFLOAT,
		R64G64B64A64_UINT,
		R64G64B64A64_SINT,
		R64G64B64A64_SFLOAT,
		B10G11R11_UFLOAT_PACK32,
		E5B9G9R9_UFLOAT_PACK32,

		D16_UNORM,
		X8_D24_UNORM_PACK32,
		D32_SFLOAT,
		S8_UINT,
		D16_UNORM_S8_UINT,
		D24_UNORM_S8_UINT,
		D32_SFLOAT_S8_UINT,

		BC1_RGB_UNORM_BLOCK,
		BC1_RGB_SRGB_BLOCK,
		BC1_RGBA_UNORM_BLOCK,
		BC1_RGBA_SRGB_BLOCK,
		BC2_UNORM_BLOCK,
		BC2_SRGB_BLOCK,
		BC3_UNORM_BLOCK,
		BC3_SRGB_BLOCK,
		BC4_UNORM_BLOCK,
		BC4_SNORM_BLOCK,
		BC5_UNORM_BLOCK,
		BC5_SNORM_BLOCK,
		BC6H_UFLOAT_BLOCK,
		BC6H_SFLOAT_BLOCK,
		BC7_UNORM_BLOCK,
		BC7_SRGB_BLOCK,
		ETC2_R8G8B8_UNORM_BLOCK,
		ETC2_R8G8B8_SRGB_BLOCK,
		ETC2_R8G8B8A1_UNORM_BLOCK,
		ETC2_R8G8B8A1_SRGB_BLOCK,
		ETC2_R8G8B8A8_UNORM_BLOCK,
		ETC2_R8G8B8A8_SRGB_BLOCK,
		EAC_R11_UNORM_BLOCK,
		EAC_R11_SNORM_BLOCK,
		EAC_R11G11_UNORM_BLOCK,
		EAC_R11G11_SNORM_BLOCK,
		ASTC_4x4_UNORM_BLOCK,
		ASTC_4x4_SRGB_BLOCK,
		ASTC_5x4_UNORM_BLOCK,
		ASTC_5x4_SRGB_BLOCK,
		ASTC_5x5_UNORM_BLOCK,
		ASTC_5x5_SRGB_BLOCK,
		ASTC_6x5_UNORM_BLOCK,
		ASTC_6x5_SRGB_BLOCK,
		ASTC_6x6_UNORM_BLOCK,
		ASTC_6x6_SRGB_BLOCK,
		ASTC_8x5_UNORM_BLOCK,
		ASTC_8x5_SRGB_BLOCK,
		ASTC_8x6_UNORM_BLOCK,
		ASTC_8x6_SRGB_BLOCK,
		ASTC_8x8_UNORM_BLOCK,
		ASTC_8x8_SRGB_BLOCK,
		ASTC_10x5_UNORM_BLOCK,
		ASTC_10x5_SRGB_BLOCK,
		ASTC_10x6_UNORM_BLOCK,
		ASTC_10x6_SRGB_BLOCK,
		ASTC_10x8_UNORM_BLOCK,
		ASTC_10x8_SRGB_BLOCK,
		ASTC_10x10_UNORM_BLOCK,
		ASTC_10x10_SRGB_BLOCK,
		ASTC_12x10_UNORM_BLOCK,
		ASTC_12x10_SRGB_BLOCK,
		ASTC_12x12_UNORM_BLOCK,
		ASTC_12x12_SRGB_BLOCK,
		G8B8G8R8_422_UNORM,
		B8G8R8G8_422_UNORM,
		G8_B8_R8_3PLANE_420_UNORM,
		G8_B8R8_2PLANE_420_UNORM,
		G8_B8_R8_3PLANE_422_UNORM,
		G8_B8R8_2PLANE_422_UNORM,
		G8_B8_R8_3PLANE_444_UNORM,
		R10X6_UNORM_PACK16,
		R10X6G10X6_UNORM_2PACK16,
		R10X6G10X6B10X6A10X6_UNORM_4PACK16,
		G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
		B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
		G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
		G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
		G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
		G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
		G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
		R12X4_UNORM_PACK16,
		R12X4G12X4_UNORM_2PACK16,
		R12X4G12X4B12X4A12X4_UNORM_4PACK16,
		G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
		B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
		G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
		G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
		G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
		G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
		G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
		G16B16G16R16_422_UNORM,
		B16G16R16G16_422_UNORM,
		G16_B16_R16_3PLANE_420_UNORM,
		G16_B16R16_2PLANE_420_UNORM,
		G16_B16_R16_3PLANE_422_UNORM,
		G16_B16R16_2PLANE_422_UNORM,
		G16_B16_R16_3PLANE_444_UNORM,
		G8_B8R8_2PLANE_444_UNORM,
		G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
		G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
		G16_B16R16_2PLANE_444_UNORM,
		A4R4G4B4_UNORM_PACK16,
		A4B4G4R4_UNORM_PACK16,
		ASTC_4x4_SFLOAT,
		ASTC_5x4_SFLOAT,
		ASTC_5x5_SFLOAT,
		ASTC_6x5_SFLOAT,
		ASTC_6x6_SFLOAT,
		ASTC_8x5_SFLOAT,
		ASTC_8x6_SFLOAT,
		ASTC_8x8_SFLOAT,
		ASTC_10x5_SFLOAT,
		ASTC_10x6_SFLOAT,
		ASTC_10x8_SFLOAT,
		ASTC_10x10_SFLOAT,
		ASTC_12x10_SFLOAT,
		ASTC_12x12_SFLOAT,
		PVRTC1_2BPP_UNORM,
		PVRTC1_4BPP_UNORM,
		PVRTC2_2BPP_UNORM,
		PVRTC2_4BPP_UNORM,
		PVRTC1_2BPP_SRGB,
		PVRTC1_4BPP_SRGB,
		PVRTC2_2BPP_SRGB,
		PVRTC2_4BPP_SRGB,

		// Limited Support:
		R16G16_S10_5,
		A1B5G5R5_UNORM_PACK16,
		A8_UNORM,
	};

	enum class FormatDepthStencilType {
		NotDepthStencil,
		DepthOnly,
		StencilOnly,
		DepthStencil
	};

	FormatDepthStencilType GetFormatDepthStencilType(Format format);
	bool IsFormatCompressed(Format format);
	uint8_t GetCompressedFormatBlockSize(Format format);
	uint8_t GetFormatBytesPerPixel(Grindstone::GraphicsAPI::Format format);

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
		None,
		Sampler,
		CombinedImageSampler,
		SampledImage,
		StorageImage,
		UniformTexelBuffer,
		StorageTexelBuffer,
		UniformBuffer,
		StorageBuffer,
		UniformBufferDynamic,
		StorageBufferDynamic,
		AccelerationStructure
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

	// This refers to semantic information about how some vertex data will be used.
	enum class AttributeUsage {
		Position,
		Color,
		TexCoord0,
		TexCoord1,
		TexCoord2,
		TexCoord3,
		Normal,
		Tangent,
		BlendWeights,
		BlendIndices,
		Other
	};

	// This refers to how the buffer should be indexed.
	enum class VertexInputRate {
		Vertex,
		Instance
	};

	// A structure to define a particular kind of data in a Vertex Buffer.
	struct VertexAttributeDescription {
		const char* name = nullptr;
		uint32_t bindingIndex;
		uint32_t locationIndex = 0;
		Format format = Format::R32_SFLOAT;
		uint32_t byteOffset = 0;
		AttributeUsage attributeUsage = AttributeUsage::Other;
	};

	// A structure that dictates how the Vertex Buffer data is formatted.
	struct VertexBindingDescription {
		uint32_t bindingIndex;
		uint32_t stride = 0;
		VertexInputRate inputRate = VertexInputRate::Vertex;
	};

	struct VertexInputLayout {
		std::vector<VertexBindingDescription> bindings;
		std::vector<VertexAttributeDescription> attributes;
	};

	struct VertexInputLayoutBuilder {
		std::vector<VertexBindingDescription> bindings;
		std::vector<VertexAttributeDescription> attributes;

		VertexInputLayoutBuilder() = default;

		struct InlineAttribute {
			const char* name;
			uint32_t locationIndex;
			Format format;
			uint32_t byteOffset;
			AttributeUsage attributeUsage;
		};

		VertexInputLayoutBuilder& AddBinding(
			VertexBindingDescription binding,
			std::initializer_list<InlineAttribute> newAttributes
		) {
			bindings.emplace_back(binding);
			for (const InlineAttribute& attrib : newAttributes) {
				attributes.emplace_back(VertexAttributeDescription{
					attrib.name,
					binding.bindingIndex,
					attrib.locationIndex,
					attrib.format,
					attrib.byteOffset,
					attrib.attributeUsage
				});
			}

			return *this;
		}

		VertexInputLayoutBuilder& AddBinding(VertexBindingDescription binding) {
			bindings.emplace_back(binding);
			return *this;
		}

		VertexInputLayoutBuilder& AddAttribute(VertexAttributeDescription attribute) {
			attributes.emplace_back(attribute);
			return *this;
		}

		VertexInputLayout Build() {
			return {
				bindings,
				attributes
			};
		}
	};
}

inline Grindstone::GraphicsAPI::ShaderStageBit ToShaderStageBit(const Grindstone::GraphicsAPI::ShaderStage stage) {
	using ShaderStageType = uint8_t;
	return static_cast<Grindstone::GraphicsAPI::ShaderStageBit>(1 << static_cast<ShaderStageType>(stage));
}

inline Grindstone::GraphicsAPI::ShaderStageBit operator~(const Grindstone::GraphicsAPI::ShaderStageBit stages) {
	using ShaderStageBitType = uint8_t;
	return static_cast<Grindstone::GraphicsAPI::ShaderStageBit>(~static_cast<ShaderStageBitType>(stages));
}

inline Grindstone::GraphicsAPI::ShaderStageBit operator|(const Grindstone::GraphicsAPI::ShaderStageBit a, const Grindstone::GraphicsAPI::ShaderStageBit b) {
	using ShaderStageBitType = uint8_t;
	return static_cast<Grindstone::GraphicsAPI::ShaderStageBit>(static_cast<ShaderStageBitType>(a) | static_cast<ShaderStageBitType>(b));
}

inline Grindstone::GraphicsAPI::ShaderStageBit operator&(const Grindstone::GraphicsAPI::ShaderStageBit a, const Grindstone::GraphicsAPI::ShaderStageBit b) {
	using ShaderStageBitType = uint8_t;
	return static_cast<Grindstone::GraphicsAPI::ShaderStageBit>(static_cast<ShaderStageBitType>(a) & static_cast<ShaderStageBitType>(b));
}

inline Grindstone::GraphicsAPI::ShaderStageBit operator^(const Grindstone::GraphicsAPI::ShaderStageBit a, const Grindstone::GraphicsAPI::ShaderStageBit b) {
	using ShaderStageBitType = uint8_t;
	return static_cast<Grindstone::GraphicsAPI::ShaderStageBit>(static_cast<ShaderStageBitType>(a) ^ static_cast<ShaderStageBitType>(b));
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
	using ColorMaskType = uint8_t;
	return static_cast<Grindstone::GraphicsAPI::ColorMask>(~static_cast<ColorMaskType>(stages));
}

inline Grindstone::GraphicsAPI::ColorMask operator|(const Grindstone::GraphicsAPI::ColorMask a, const Grindstone::GraphicsAPI::ColorMask b) {
	using ColorMaskType = uint8_t;
	return static_cast<Grindstone::GraphicsAPI::ColorMask>(static_cast<ColorMaskType>(a) | static_cast<ColorMaskType>(b));
}

inline Grindstone::GraphicsAPI::ColorMask operator&(const Grindstone::GraphicsAPI::ColorMask a, const Grindstone::GraphicsAPI::ColorMask b) {
	using ColorMaskType = uint8_t;
	return static_cast<Grindstone::GraphicsAPI::ColorMask>(static_cast<ColorMaskType>(a) & static_cast<ColorMaskType>(b));
}

inline Grindstone::GraphicsAPI::ColorMask operator^(const Grindstone::GraphicsAPI::ColorMask a, const Grindstone::GraphicsAPI::ColorMask b) {
	using ColorMaskType = uint8_t;
	return static_cast<Grindstone::GraphicsAPI::ColorMask>(static_cast<ColorMaskType>(a) ^ static_cast<ColorMaskType>(b));
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

namespace std {
	template<>
	struct std::hash<Grindstone::GraphicsAPI::VertexBindingDescription> {
		std::size_t operator()(const Grindstone::GraphicsAPI::VertexBindingDescription& binding) const noexcept {
			size_t result = std::hash<size_t>{}(
				static_cast<size_t>(binding.bindingIndex) << 8 |
				static_cast<size_t>(binding.stride) << 32
				);
			result ^= std::hash<size_t>{}(static_cast<size_t>(binding.inputRate));
			return result;
		}
	};

	template<>
	struct std::hash<Grindstone::GraphicsAPI::VertexAttributeDescription> {
		std::size_t operator()(const Grindstone::GraphicsAPI::VertexAttributeDescription& attribute) const noexcept {
			size_t result = std::hash<size_t>{}(
				static_cast<size_t>(attribute.attributeUsage) |
				static_cast<size_t>(attribute.bindingIndex) << 32
				);

			result ^= std::hash<size_t>{}(
				static_cast<size_t>(attribute.byteOffset) |
				static_cast<size_t>(attribute.format) << 32
				);

			result ^= std::hash<size_t>{}(
				static_cast<size_t>(attribute.locationIndex) << 32
				);

			return result;
		}
	};

	template<>
	struct std::hash<Grindstone::GraphicsAPI::VertexInputLayout> {
		std::size_t operator()(const Grindstone::GraphicsAPI::VertexInputLayout& vertexInputLayout) const noexcept {
			size_t result = std::hash<size_t>{}(vertexInputLayout.attributes.size()) ^ std::hash<size_t>{}(vertexInputLayout.bindings.size());

			for (const Grindstone::GraphicsAPI::VertexBindingDescription& binding : vertexInputLayout.bindings) {
				result ^= std::hash<Grindstone::GraphicsAPI::VertexBindingDescription>{}(binding);
			}

			for (const Grindstone::GraphicsAPI::VertexAttributeDescription& attribute : vertexInputLayout.attributes) {
				result ^= std::hash<Grindstone::GraphicsAPI::VertexAttributeDescription>{}(attribute);
			}

			return result;
		}
	};
}
