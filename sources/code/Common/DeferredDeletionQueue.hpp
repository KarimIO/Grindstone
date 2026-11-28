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
		DeletionQueue() = default;
		DeletionQueue(uint32_t poolSize);
		DeletionQueue(const DeletionQueue& other) = default;
		DeletionQueue(DeletionQueue&& other) noexcept = default;
		DeletionQueue& operator=(const DeletionQueue& other) = default;
		DeletionQueue& operator=(DeletionQueue&& other) noexcept = default;
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
		DeferredDeletionQueue() = default;
		DeferredDeletionQueue(uint8_t frameCount, uint32_t poolSize);
		DeferredDeletionQueue(const DeferredDeletionQueue& other) = default;
		DeferredDeletionQueue(DeferredDeletionQueue&& other) noexcept = default;
		DeferredDeletionQueue& operator=(const DeferredDeletionQueue& other) = default;
		DeferredDeletionQueue& operator=(DeferredDeletionQueue&& other) noexcept = default;
		void Initialize(uint8_t frameCount, uint32_t poolSize);
		void PushDeletion(std::function<void()> fn);
		void DeleteAll();
		void DeleteForFrame();
		void SetCurrentImageIndex(size_t newFrameIndex);
	protected:
		std::vector<DeletionQueue> queues;
		size_t frameIndex = 0;
	};
}
