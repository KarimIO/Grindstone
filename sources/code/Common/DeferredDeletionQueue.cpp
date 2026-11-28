#include <EngineCore/EngineCore.hpp>
#include <Common/Window/WindowManager.hpp>
#include <Common/Graphics/WindowGraphicsBinding.hpp>

#include "DeferredDeletionQueue.hpp"

Grindstone::DeletionQueue::DeletionQueue(uint32_t poolSize) {
	Initialize(poolSize);
}

Grindstone::DeletionQueue::~DeletionQueue() {
	DeleteAll();
	queue.clear();
}

void Grindstone::DeletionQueue::Initialize(uint32_t poolSize) {
	queue.reserve(poolSize);
}

void Grindstone::DeletionQueue::PushDeletion(std::function<void()> fn) {
	queue.emplace_back(fn);
}

void Grindstone::DeletionQueue::DeleteAll() {
	for (auto& deletable : queue) {
		deletable();
	}

	queue.clear();
}

Grindstone::DeferredDeletionQueue::~DeferredDeletionQueue() {
	DeleteAll();
	queues.clear();
}

Grindstone::DeferredDeletionQueue::DeferredDeletionQueue(uint8_t frameCount, uint32_t poolSize) {
	Initialize(frameCount, poolSize);
}

void Grindstone::DeferredDeletionQueue::Initialize(uint8_t frameCount, uint32_t poolSize) {
	if (queues.size() > frameCount) {
		for (size_t i = frameCount; i < queues.size(); ++i) {
			queues[i].DeleteAll();
		}
	}

	queues.resize(frameCount);

	for (size_t i = 0; i < queues.size(); ++i) {
		queues[i].Initialize(poolSize);
	}
}

void Grindstone::DeferredDeletionQueue::SetCurrentImageIndex(size_t newFrameIndex) {
	frameIndex = newFrameIndex;
}

void Grindstone::DeferredDeletionQueue::PushDeletion(std::function<void()> fn) {
	queues[frameIndex].PushDeletion(fn);
}

void Grindstone::DeferredDeletionQueue::DeleteAll() {
	for (size_t i = 0; i < queues.size(); ++i) {
		queues[i].DeleteAll();
	}
}

void Grindstone::DeferredDeletionQueue::DeleteForFrame() {
	queues[frameIndex].DeleteAll();
}
