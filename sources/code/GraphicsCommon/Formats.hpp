#pragma once

enum ColorFormat {
	FORMAT_COLOR_R8 = 0,
	FORMAT_COLOR_R8G8,
	FORMAT_COLOR_R8G8B8,
	FORMAT_COLOR_R8G8B8A8,

	FORMAT_COLOR_R16G16B16,
	FORMAT_COLOR_R16G16B16A16,
	FORMAT_COLOR_R32G32B32,
	FORMAT_COLOR_R32G32B32A32,

	FORMAT_COLOR_RGB_DXT1,
	FORMAT_COLOR_RGBA_DXT1,
	FORMAT_COLOR_RGBA_DXT3,
	FORMAT_COLOR_RGBA_DXT5,

	FORMAT_COLOR_SRGB_DXT1,
	FORMAT_COLOR_SRGB_ALPHA_DXT1,
	FORMAT_COLOR_SRGB_ALPHA_DXT3,
	FORMAT_COLOR_SRGB_ALPHA_DXT5,
};

enum DepthFormat {
	FORMAT_DEPTH_NONE = 0,
	FORMAT_DEPTH_16,
	FORMAT_DEPTH_24,
	FORMAT_DEPTH_32,
	//FORMAT_DEPTH_16_STENCIL_8,
	FORMAT_DEPTH_24_STENCIL_8,
	FORMAT_DEPTH_32_STENCIL_8
	//FORMAT_STENCIL_8
};

enum {
	SHADER_STAGE_VERTEX_BIT = 0x00000001,
	SHADER_STAGE_TESSELLATION_CONTROL_BIT = 0x00000002,
	SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 0x00000004,
	SHADER_STAGE_GEOMETRY_BIT = 0x00000008,
	SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
	SHADER_STAGE_COMPUTE_BIT = 0x00000020,
	SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,
	SHADER_STAGE_ALL = 0x7FFFFFFF
};