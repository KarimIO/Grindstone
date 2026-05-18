#pragma once

#include <vector>
#include <stdint.h>

namespace Grindstone::Renderer {
	using PassId = uint16_t;
	using ResourceId = uint16_t;
	const PassId invalidPassId = std::numeric_limits<PassId>().max();
	const ResourceId invalidResourceId = std::numeric_limits<ResourceId>().max();

	struct RenderGraphResourceRef {
		uint16_t isBuffer : 1;
		uint16_t resourceIndex : 15;

		static_assert(sizeof(uint16_t) == 2);

		static RenderGraphResourceRef Buffer(uint16_t resourceIndex) {
			return {
				.isBuffer = 1,
				.resourceIndex = resourceIndex
			};
		}

		static RenderGraphResourceRef Image(uint16_t resourceIndex) {
			return {
				.isBuffer = 0,
				.resourceIndex = resourceIndex
			};
		}

		bool IsBuffer() const {
			return isBuffer == 1;
		}

		bool IsImage() const {
			return isBuffer == 0;
		}

		uint16_t GetResourceIndex() const {
			return resourceIndex;
		}

		bool operator==(const RenderGraphResourceRef& o) const {
			return
				(isBuffer == o.isBuffer) &&
				(resourceIndex == o.resourceIndex);
		}
	};

	struct RenderGraphBuilderResourceRef {
		ResourceId isBuffer : 1;
		ResourceId resourceIndex : 15;
		PassId passIndex;

		static_assert(sizeof(ResourceId) == 2);

		static RenderGraphBuilderResourceRef Invalid() {
			return {
				.isBuffer = 0,
				.resourceIndex = static_cast<uint16_t>(0xffff),
				.passIndex = 0xffff
			};
		}

		static RenderGraphBuilderResourceRef Buffer(ResourceId resourceIndex, PassId passIndex) {
			return {
				.isBuffer = 1,
				.resourceIndex = resourceIndex,
				.passIndex = passIndex
			};
		}

		static RenderGraphBuilderResourceRef Image(ResourceId resourceIndex, PassId passIndex) {
			return {
				.isBuffer = 0,
				.resourceIndex = resourceIndex,
				.passIndex = passIndex
			};
		}

		bool IsInvalid() const {
			return resourceIndex >= 32767;
		}

		bool IsBuffer() const {
			return isBuffer == 1;
		}

		bool IsImage() const {
			return isBuffer == 0;
		}

		ResourceId GetResourceIndex() const {
			return resourceIndex;
		}

		PassId GetPassIndex() const {
			return passIndex;
		}

		RenderGraphBuilderResourceRef FromPass(PassId newPassIndex) const {
			return {
				.isBuffer = isBuffer,
				.resourceIndex = resourceIndex,
				.passIndex = newPassIndex
			};
		}

		bool IsSameResource(const RenderGraphBuilderResourceRef& other) const {
			return
				isBuffer == other.isBuffer &&
				resourceIndex == other.resourceIndex;
		}

		bool operator==(const RenderGraphBuilderResourceRef& o) const {
			return
				(isBuffer == o.isBuffer) &&
				(resourceIndex == o.resourceIndex) &&
				(passIndex == o.passIndex);
		}
	};

	enum class AccessType {
		Read,
		Write,
		ReadWrite
	};

	struct PassBufferDesc {
		RenderGraphBuilderResourceRef ref;
		AccessType accessType;
	};
}

template<>
struct std::hash<Grindstone::Renderer::RenderGraphBuilderResourceRef> {
	size_t operator()(const Grindstone::Renderer::RenderGraphBuilderResourceRef& h) const {
		// Pack the bitfields + passIndex into a single integer to hash
		uint32_t packed = (h.isBuffer << 15) | h.resourceIndex;

		return std::hash<uint64_t>{}(
			(static_cast<uint64_t>(packed) << 16) | h.passIndex
		);
	}
};
