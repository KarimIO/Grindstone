#pragma once

#include <vector>

namespace Grindstone {
	namespace Events {
		class IEventListener;
		struct BaseEvent;

		class EventCoordinator {
		public:
			void RegisterEventListener(IEventListener*);
			void EnqueueEvent(BaseEvent*);
			void HandleEvents();
		private:
			void HandleEvent(BaseEvent* eventToHandle);
		private:
			std::vector<IEventListener*> eventListeners;
			std::vector<BaseEvent*> eventsToHandle;
		}; // class EventCoordinator
	} // namespace Events
} // namespace Grindstone
