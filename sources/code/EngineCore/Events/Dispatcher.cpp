#include <functional>
#include "Common/Event/EventType.hpp"
#include "Common/Event/BaseEvent.hpp"
#include "Dispatcher.hpp"

using namespace Grindstone::Events;

Dispatcher::Dispatcher() {
	for (size_t i = 0; i < (size_t)Events::EventType::Last; i++) {
		eventListeners[(Events::EventType)i] = (EventListenerList*)new std::vector<EventListener*>();
	}
}

void Dispatcher::AddEventListener(EventType eventType, std::function<bool(BaseEvent*)> function) {
	eventListeners[eventType]->emplace_back(function);
}

void Dispatcher::Dispatch(BaseEvent* event) {
	eventsToHandle.emplace_back(event);
}

void Dispatcher::HandleEvents() {
	for (BaseEvent* eventToHandle : eventsToHandle) {
		HandleEvent(eventToHandle);
	}

	eventsToHandle.clear();
}

void Dispatcher::HandleEvent(BaseEvent* eventToHandle) {
	EventListenerList* eventListenerList = eventListeners[eventToHandle->GetEventType()];
	for (auto& eventCallback : *eventListenerList) {
		bool isEventHandled = eventCallback(eventToHandle);

		if (isEventHandled) {
			return;
		}
	}

	delete eventToHandle;
	eventToHandle = nullptr;
}
