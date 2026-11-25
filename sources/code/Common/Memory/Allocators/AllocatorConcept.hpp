#pragma once

#include <string>
#include <functional>
#include <stdint.h>
#include <map>
#include <utility>

#include "../SmartPointers.hpp"

namespace Grindstone::Memory::Allocators {
	/**
	 * \brief An allocator concept that can allocate and deallocate.
	 */
	template <typename T> 
	concept Allocator = requires(T a, typename T::value_type* p) {
		{ a.allocate(0) } -> std::same_as<decltype(p)>;
		{ a.deallocate(p, 0) } -> std::same_as<void>;
	};
}
