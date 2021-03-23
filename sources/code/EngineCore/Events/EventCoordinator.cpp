#include "EventCoordinator.hpp"
#include "IEventListener.hpp"

using namespace Grindstone::Events;

void EventCoordinator::RegisterEventListener(IEventListener* eventListener) {
	eventListeners.emplace_back(eventListener);
}

void EventCoordinator::EnqueueEvent(BaseEvent* event) {
	eventsToHandle.emplace_back(event);
}

void EventCoordinator::HandleEvents() {
	for (BaseEvent* eventToHandle : eventsToHandle) {
		HandleEvent(eventToHandle);
	}

	eventsToHandle.clear();
}

void EventCoordinator::HandleEvent(BaseEvent* eventToHandle) {
	for (IEventListener* eventListener : eventListeners) {
		bool isEventHandled = eventListener->HandleEvent(eventToHandle);

		if (isEventHandled) {
			return;
		}
	}

	delete eventToHandle;
	eventToHandle = nullptr;
}
