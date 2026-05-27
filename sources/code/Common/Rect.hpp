#pragma once
#include "Math.hpp"

namespace Grindstone::Math {

	using Offset2D = Grindstone::Math::Int2;
	using Offset3D = Grindstone::Math::Int3;
	using Extent2D = Grindstone::Math::Uint2;
	using Extent3D = Grindstone::Math::Uint3;

	struct Rect2D {
		Rect2D() = default;
		Rect2D(const Rect2D& other) = default;
		Rect2D(Rect2D&& other) noexcept = default;
		Rect2D& operator=(const Rect2D& other) = default;
		Rect2D& operator=(Rect2D&& other) noexcept = default;

		Rect2D(float width, float height) : offset(0.0f, 0.0f), extent(width, height) {}
		Rect2D(float x, float y, float width, float height) : offset(x, y), extent(width, height) {}
		Rect2D(Grindstone::Math::Float2 extent) : offset(0, 0), extent(extent) {}
		Rect2D(Grindstone::Math::Float2 offset, Grindstone::Math::Float2 extent) : offset(offset), extent(extent) {}
		Rect2D(Grindstone::Math::Float4 v) : offset(v.x, v.y), extent(v.z, v.w) {}

		float GetX() const { return offset.x; }
		float GetY() const { return offset.y; }
		float GetWidth() const { return extent.x; }
		float GetHeight() const { return extent.y; }

		Grindstone::Math::Float2 offset;
		Grindstone::Math::Float2 extent;
	};

	struct IntRect2D {
		IntRect2D() = default;
		IntRect2D(const IntRect2D& other) = default;
		IntRect2D(IntRect2D&& other) noexcept = default;
		IntRect2D& operator=(const IntRect2D& other) = default;
		IntRect2D& operator=(IntRect2D&& other) noexcept = default;

		IntRect2D(uint32_t width, uint32_t height) : offset(Grindstone::Math::Offset2D(0, 0)), extent(Grindstone::Math::Extent2D(width, height)) {}
		IntRect2D(int32_t x, int32_t y, uint32_t width, uint32_t height) : offset(Grindstone::Math::Offset2D(x, y)), extent(Grindstone::Math::Extent2D(width, height)) {}
		IntRect2D(Grindstone::Math::Extent2D extent) : offset(Grindstone::Math::Offset2D(0, 0)), extent(extent) {}
		IntRect2D(Grindstone::Math::Offset2D offset, Grindstone::Math::Extent2D extent) : offset(offset), extent(extent) {}

		int32_t GetX() const { return offset.x; }
		int32_t GetY() const { return offset.y; }
		uint32_t GetWidth() const { return extent.x; }
		uint32_t GetHeight() const { return extent.y; }

		Grindstone::Math::Offset2D offset;
		Grindstone::Math::Extent2D extent;
	};

	struct Box3D {
		Box3D() = default;
		Box3D(const Box3D& other) = default;
		Box3D(Box3D&& other) noexcept = default;
		Box3D& operator=(const Box3D& other) = default;
		Box3D& operator=(Box3D&& other) noexcept = default;

		Box3D(float width, float height, float depth) : offset(0.0f, 0.0f, 0.0f), extent(width, height, depth) {}
		Box3D(Grindstone::Math::Float3 extent) : offset(0.0f, 0.0f, 0.0f), extent(extent) {}
		Box3D(float x, float y, float z, float width, float height, float depth) : offset(x, y, z), extent(width, height, depth) {}
		Box3D(Grindstone::Math::Float3 offset, Grindstone::Math::Float3 extent) : offset(offset), extent(extent) {}

		float GetX() const { return offset.x; }
		float GetY() const { return offset.y; }
		float GetZ() const { return offset.z; }
		float GetWidth() const { return extent.x; }
		float GetHeight() const { return extent.y; }
		float GetDepth() const { return extent.z; }

		Grindstone::Math::Float3 offset;
		Grindstone::Math::Float3 extent;
	};

	struct IntBox3D {
		IntBox3D() = default;
		IntBox3D(const IntBox3D& other) = default;
		IntBox3D(IntBox3D&& other) noexcept = default;
		IntBox3D& operator=(const IntBox3D& other) = default;
		IntBox3D& operator=(IntBox3D&& other) noexcept = default;

		IntBox3D(uint32_t width, uint32_t height, uint32_t depth) : offset(Grindstone::Math::Offset3D(0, 0, 0)), extent(Grindstone::Math::Extent3D(width, height, depth)) {}
		IntBox3D(Grindstone::Math::Extent3D extent) : offset(Grindstone::Math::Offset3D(0, 0, 0)), extent(extent) {}
		IntBox3D(int32_t x, int32_t y, int32_t z, uint32_t width, uint32_t height, uint32_t depth) : offset(Grindstone::Math::Offset3D(x, y, z)), extent(Grindstone::Math::Extent3D(width, height, depth)) {}
		IntBox3D(Grindstone::Math::Offset3D offset, Grindstone::Math::Extent3D extent) : offset(offset), extent(extent) {}

		int32_t GetX() const { return offset.x; }
		int32_t GetY() const { return offset.y; }
		int32_t GetZ() const { return offset.z; }
		uint32_t GetWidth() const { return extent.x; }
		uint32_t GetHeight() const { return extent.y; }
		uint32_t GetDepth() const { return extent.z; }

		Grindstone::Math::Offset3D offset;
		Grindstone::Math::Extent3D extent;
	};

}
