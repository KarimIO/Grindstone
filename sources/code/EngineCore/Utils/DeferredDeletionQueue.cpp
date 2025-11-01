#include <EngineCore/EngineCore.hpp>
#include <Common/Window/WindowManager.hpp>
#include <Common/Graphics/WindowGraphicsBinding.hpp>

#include "DeferredDeletionQueue.hpp"

Grindstone::DeletionQueue::~DeletionQueue() {
	DeleteAll();
	queue.clear();
}

void Grindstone::DeletionQueue::Initialize(uint8_t poolSize) {
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

void Grindstone::DeferredDeletionQueue::PushDeletion(std::function<void()> fn) {
	size_t frameIndex = EngineCore::GetInstance().windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding()->GetCurrentImageIndex();
	queues[frameIndex].PushDeletion(fn);
}

void Grindstone::DeferredDeletionQueue::DeleteAll() {
	for (size_t i = 0; i < queues.size(); ++i) {
		queues[i].DeleteAll();
	}
}

void Grindstone::DeferredDeletionQueue::DeleteForFrame() {
	size_t frameIndex = EngineCore::GetInstance().windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding()->GetCurrentImageIndex();
	queues[frameIndex].DeleteAll();
}
