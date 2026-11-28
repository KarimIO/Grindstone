#pragma once

#include <string>
#include <functional>
#include <stdint.h>
#include <map>
#include <utility>

#include "../SmartPointers.hpp"

namespace Grindstone::Memory::Allocators {
	/**
	 * \brief An allocator concept that can allocate but not deallocate.
	 */
	template <typename T> 
	concept AllocatorOnly = requires(T a) {
		{ a.AllocateRaw(std::size_t{}, std::size_t{}, "") } -> std::same_as<void*>;
	};
	
	/**
	 * \brief An allocator concept that can allocate and deallocate.
	 */
	template <typename T> 
	concept Allocator = AllocatorOnly<T> && requires(T a) {
		{ a.Free(nullptr) } -> std::same_as<bool>;
	};
}
