#pragma once

#include <stdint.h>

namespace Grindstone {
	namespace GraphicsAPI {
		enum class ColorFormat : uint8_t {
			R8 = 0,
			R8G8,
			R8G8B8,
			R8G8B8A8,

			R10G10B10A2,

			R16,
			R16G16,
			R16G16B16,
			R16G16B16A16,
			R32G32B32,
			R32G32B32A32,

			RGB_DXT1,
			RGBA_DXT1,
			RGBA_DXT3,
			RGBA_DXT5,

			SRGB_DXT1,
			SRGB_ALPHA_DXT1,
			SRGB_ALPHA_DXT3,
			SRGB_ALPHA_DXT5,
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
			Compute,
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
	};
};