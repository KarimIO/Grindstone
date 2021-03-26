#pragma once

#include "Common/Event/MouseButtonCode.hpp"

struct ImGuiIO;

namespace Grindstone {
	class EngineCore;
	
	namespace Events {
		struct BaseEvent;
		struct MouseMovedEvent;
		struct MousePressEvent;
	}

	namespace Editor {
		namespace ImguiEditor {
			class ImguiInput {
			public:
				ImguiInput(ImGuiIO& io, EngineCore* engineCore);
			private:
				ImGuiIO& io;
			};
		}
	}
}