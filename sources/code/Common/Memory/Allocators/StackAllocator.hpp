#pragma once

namespace Grindstone::Memory::Allocators {
	/**
	 * \brief A stack allocator where memory can only be deallocated in order (LIFO).
	 *
	 * Memory in a stack allocator is allocated sequentially, and can only be deallocated
	 * in reverse order (LIFO).
	 *
	 */
	class StackAllocator {
	public:
		bool Initialize(size_t size);
		bool Allocate(size_t size);
	};
}
