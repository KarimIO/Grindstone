#pragma once

#include <stdint.h>

enum class ParameterType : uint8_t {
	Color,
	Bool,
	Int,
	Int2,
	Int3,
	Int4,
	Uint,
	Uint2,
	Uint3,
	Uint4,
	Float,
	Float2,
	Float3,
	Float4,
	Double,
	Double2,
	Double3,
	Double4,
	Matrix2x2,
	Matrix2x3,
	Matrix2x4,
	Matrix3x2,
	Matrix4x2,
	Matrix3x3,
	Matrix3x4,
	Matrix4x3,
	Matrix4x4,
	Texture,
	Sampler,
	Image,
	Atomic
};
