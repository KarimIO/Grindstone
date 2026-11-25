#pragma once

#include <functional>
#include <vector>

namespace Grindstone {
	/**
	 * \brief A dynamic allocator represented by a linked list.
	 *
	 * Memory in a dynamic allocator can be allocated and deallocated freely, with no restrictions.
	 * Allocations have a header that points to the next block of memory, and so free memory can be found
	 * in between allocated blocks of memory.
	 */
	class DeletionQueue {
	public:
		~DeletionQueue();
		void Initialize(uint32_t poolSize);
		void PushDeletion(std::function<void()> fn);
		void DeleteAll();
	protected:
		std::vector<std::function<void()>> queue;
	};

	/**
	 * \brief A series of per-frame DeletionQueues that allow the game to delete resources all at once.
	 *
	 * Memory in a dynamic allocator can be allocated and deallocated freely,
	 * with no restrictions. Allocations have a header that points to the next
	 * block of memory, and so free memory can be found in between allocated
	 * blocks of memory.
	 */
	class DeferredDeletionQueue {
	public:
		~DeferredDeletionQueue();
		void Initialize(uint8_t frameCount, uint32_t poolSize);
		void PushDeletion(std::function<void()> fn);
		void DeleteAll();
		void DeleteForFrame();
	protected:
		std::vector<DeletionQueue> queues;
	};
}
