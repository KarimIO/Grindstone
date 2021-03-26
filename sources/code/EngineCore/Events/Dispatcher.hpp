#pragma once

#include "Common/Event/EventType.hpp"
#include <vector>
#include <map>

namespace Grindstone {
	namespace Events {
		struct BaseEvent;

		using EventListener = bool(BaseEvent*);

		class Dispatcher {
		public:
			struct EventCallback {
				EventCallback(
					bool (*fn)(BaseEvent*, void*),
					void* data
				) : fn(fn), data(data) {}
				bool (*fn)(BaseEvent*, void*);
				void* data;
			};

			Dispatcher();
			virtual void AddEventListener(EventType eventType, bool (*functionCallback)(BaseEvent*, void*), void* data);
			virtual void Dispatch(BaseEvent*);
			virtual void HandleEvents();
		private:
			void HandleEvent(BaseEvent* eventToHandle);
		private:
			using EventListenerList = std::vector<EventCallback>;
			std::map<Events::EventType, EventListenerList*> eventListeners;
			std::vector<BaseEvent*> eventsToHandle;
		}; // class Dispatcher
	} // namespace Events
} // namespace Grindstone
