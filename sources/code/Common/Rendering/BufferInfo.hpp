#pragma once

#include <vector>
#include <string>
#include <map>
#include <stdint.h>

#include <Common/Graphics/Buffer.hpp>
#include <Common/Graphics/Formats.hpp>

namespace Grindstone::Renderer {
	struct BufferDescription {
		Grindstone::String name;
		size_t size = 0;
		Grindstone::GraphicsAPI::BufferUsage bufferUsage;
		Grindstone::GraphicsAPI::MemoryUsage memoryUsage;

		bool operator==(const BufferDescription& other) const {
			return size == other.size &&
				bufferUsage == other.bufferUsage &&
				memoryUsage == other.memoryUsage;
		}

		bool operator!=(const BufferDescription& other) const {
			return !(*this == other);
		}
	};
}

namespace std {
	template<>
	struct std::hash<Grindstone::Renderer::BufferDescription> {
		std::size_t operator()(const Grindstone::Renderer::BufferDescription& desc) const noexcept {
			size_t result = std::hash<size_t>{}(
				static_cast<size_t>(desc.bufferUsage) |
				static_cast<size_t>(desc.memoryUsage) << 32
				);

			result ^= std::hash<size_t>{}(static_cast<size_t>(desc.size));
			return result;
		}
	};
}
