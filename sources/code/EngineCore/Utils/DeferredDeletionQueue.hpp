#pragma once

#include <vector>
#include <queue>

namespace Grindstone {
	class DeletionQueue {
	public:
		~DeletionQueue();
		void Initialize(uint8_t poolSize);
		void PushDeletion(std::function<void()> fn);
		void DeleteAll();
	protected:
		std::vector<std::function<void()>> queue;
	};

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
