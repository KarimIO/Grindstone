#pragma once

#include "EventType.hpp"

namespace Grindstone {
	namespace Events {
		struct BaseEvent {			
			virtual EventType GetEventType() const = 0;
			virtual const char* GetName() const = 0;
			virtual const char* ToString() const {
				return GetName();
			};
		}; // struct BaseEvent
	} // namespace Events
} // namespace Grindstone

#define SETUP_EVENT(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return EventType::type; }\
								virtual const char* GetName() const override { return #type; }
