#pragma once

#include <string>
#include <functional>
#include <stdint.h>
#include <map>
#include <utility>

#include "../SmartPointers.hpp"

namespace Grindstone::Memory::Allocators {
	/**
	 * \brief A dynamic allocator represented by a linked list.
	 *
	 * Memory in a dynamic allocator can be allocated and deallocated freely, with no restrictions.
	 * Allocations have a header that points to the next block of memory, and so free memory can be found
	 * in between allocated blocks of memory.
	 */
	template <typename Allocator> 
	concept is_allocator = requires(Allocator a, typename Allocator::value_type* p) {
		{ a.allocate(0) } -> std::same_as<decltype(p)>;
		{ a.deallocate(p, 0) } -> std::same_as<void>;
	};
}
