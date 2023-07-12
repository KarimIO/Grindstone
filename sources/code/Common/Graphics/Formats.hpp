#pragma once

#include <stdint.h>

namespace Grindstone {
	namespace GraphicsAPI {
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
			DepthTexture
		};

		const uint8_t numShaderGraphicStage = 5;
		const uint8_t numShaderTotalStage = 6;

		enum class BlendMode : uint8_t {
			None = 0,
			Additive,
			AdditiveAlpha,
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

		enum class ColorMask : uint8_t {
			Red = 0x1,
			Green = 0x2,
			Blue = 0x4,
			Alpha = 0x8,
			RG = Red | Green,
			RGB = RG | Blue,
			RGBA = RGB | Alpha
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
	};
};
