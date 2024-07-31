#include <functional>
#include "Common/Event/EventType.hpp"
#include "Common/Event/BaseEvent.hpp"
#include "Dispatcher.hpp"

using namespace Grindstone::Events;

Dispatcher::Dispatcher() {
	for (size_t i = 0; i < (size_t)Events::EventType::Last; i++) {
		eventListeners[(Events::EventType)i] = (EventListenerList)std::vector<EventListener>();
	}
}

void Dispatcher::AddEventListener(EventType eventType, std::function<bool(BaseEvent*)> function) {
	eventListeners[eventType].emplace_back(function);
}

void Dispatcher::Dispatch(BaseEvent* event) {
	eventsToHandle.emplace_back(event);
}

void Dispatcher::HandleEvents() {
	for (int i = static_cast<int>(eventsToHandle.size() - 1); i >= 0; --i) {
		EventListenerList& eventListenerList = eventListeners[eventsToHandle[i]->GetEventType()];
		for (auto& eventCallback : eventListenerList) {
			bool isEventHandled = eventCallback(eventsToHandle[i]);

			if (isEventHandled) {
				break;
			}
		}

		delete eventsToHandle[i];
		eventsToHandle[i] = nullptr;
	}

	eventsToHandle.clear();
}

void Dispatcher::HandleEvent(BaseEvent* eventToHandle) {
	EventListenerList& eventListenerList = eventListeners[eventToHandle->GetEventType()];
	for (auto& eventCallback : eventListenerList) {
		bool isEventHandled = eventCallback(eventToHandle);

		if (isEventHandled) {
			return;
		}
	}

	delete eventToHandle;
	eventToHandle = nullptr;
}
