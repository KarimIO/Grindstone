#pragma once

#include "Common/Event/EventType.hpp"
#include <vector>
#include <functional>
#include <map>

namespace Grindstone {
	namespace Events {
		struct BaseEvent;

		using EventListener = bool(BaseEvent*);

		class Dispatcher {
		public:
			/*struct EventCallback {
				EventCallback(
					bool (*fn)(BaseEvent*, void*),
					void* data
				) : fn(fn), data(data) {}
				bool (*fn)(BaseEvent*, void*);
				void* data;
			};*/

			Dispatcher();
			virtual void AddEventListener(EventType eventType, std::function<bool(BaseEvent*)> function);
			virtual void Dispatch(BaseEvent*);
			virtual void HandleEvents();
		private:
			void HandleEvent(BaseEvent* eventToHandle);
		private:
			using EventListenerList = std::vector<std::function<bool(BaseEvent*)>>;
			std::map<Events::EventType, EventListenerList*> eventListeners;
			std::vector<BaseEvent*> eventsToHandle;
		}; // class Dispatcher
	} // namespace Events
} // namespace Grindstone
