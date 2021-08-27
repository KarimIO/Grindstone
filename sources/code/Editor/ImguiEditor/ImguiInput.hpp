#pragma once

#include "Common/Event/MouseButtonCode.hpp"

struct ImGuiIO;

namespace Grindstone {
	class EngineCore;
	
	namespace Events {
		struct BaseEvent;
	}

	namespace Editor {
		namespace ImguiEditor {
			class ImguiInput {
			public:
				ImguiInput(ImGuiIO& io, EngineCore* engineCore);
				bool OnMouseMove(Events::BaseEvent* ev);
				bool OnMousePressed(Events::BaseEvent* ev);
				bool OnMouseScrolled(Events::BaseEvent* ev);
				bool OnKeyPressed(Events::BaseEvent* ev);
				bool OnCharacterTyped(Events::BaseEvent* ev);
			private:
				ImGuiIO& io;
			};
		}
	}
}