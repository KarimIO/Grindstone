#pragma once

#include <stdint.h>
#include "Assert.hpp"

namespace Grindstone::Physics {
	constexpr uint32_t MaxLayerCount = 32;

	struct LayerMask;

	struct Layer {
		Layer() = default;
		Layer(const Layer& other) = default;
		Layer(Layer&& other) noexcept = default;
		Layer& operator=(const Layer& other) = default;
		Layer& operator=(Layer&& other) noexcept = default;

		Layer(uint8_t value) : layer(value) {
			GS_ASSERT(value < MaxLayerCount);
		}

		Layer& operator=(uint8_t value) {
			GS_ASSERT(value < MaxLayerCount);
			layer = value;
			return *this;
		}

		explicit operator uint8_t() const {
			return layer;
		}

		LayerMask GetLayerMask() const;

		uint8_t AsUint8() const {
			return layer;
		}

		uint8_t layer;
	};

	struct LayerMask {
		LayerMask() = default;
		LayerMask(const LayerMask& other) = default;
		LayerMask(LayerMask&& other) noexcept = default;
		LayerMask& operator=(const LayerMask& other) = default;
		LayerMask& operator=(LayerMask&& other) noexcept = default;

		LayerMask(uint32_t value) : mask(value) {}
		LayerMask& operator=(uint32_t value) {
			mask = value;
			return *this;
		}

		LayerMask(Layer layer) : mask(layer.GetLayerMask().AsUint32()) {}
		LayerMask& operator=(Layer layer) {
			mask = layer.GetLayerMask().AsUint32();
			return *this;
		}

		LayerMask& operator&=(const LayerMask& other) {
			mask &= other.mask;
			return *this;
		}

		LayerMask& operator|=(const LayerMask& other) {
			mask |= other.mask;
			return *this;
		}

		LayerMask& operator^=(const LayerMask& other) {
			mask ^= other.mask;
			return *this;
		}

		LayerMask operator&(const LayerMask& other) const {
			return mask & other.mask;
		}

		LayerMask operator|(const LayerMask& other) const {
			return mask | other.mask;
		}

		LayerMask operator^(const LayerMask& other) const {
			return mask ^ other.mask;
		}

		friend LayerMask operator~(const LayerMask& obj) {
			return ~obj.mask;
		}

		explicit operator uint32_t() const {
			return mask;
		}

		explicit operator bool() const {
			return mask != 0;
		}

		uint32_t AsUint32() const {
			return mask;
		}

		bool Matches(Layer layer) const {
			return mask & layer.GetLayerMask().mask;
		}

		bool Matches(LayerMask otherMask) const {
			return mask & otherMask.mask;
		}

		bool HasValue() const {
			return mask != 0;
		}

		uint32_t mask;
	};

	inline LayerMask Layer::GetLayerMask() const {
		return LayerMask(1 << layer);
	}
}
