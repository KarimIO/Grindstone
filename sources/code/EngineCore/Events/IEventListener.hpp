#pragma once

namespace Grindstone {
	namespace Events {
		struct BaseEvent;
		
		class IEventListener {
		public:
			virtual bool HandleEvent(BaseEvent* event) = 0;
		}; // class IEventListener
	} // namespace Events
} // namespace Grindstone
