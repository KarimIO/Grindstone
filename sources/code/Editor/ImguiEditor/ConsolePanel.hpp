#pragma once

#include <deque>
#include "Common/Logging.hpp"

namespace Grindstone {
	namespace Events {
		struct BaseEvent;
	}

	namespace ECS {
		class SystemRegistrar;
	}

	namespace Editor {
		namespace ImguiEditor {
			class ImguiRenderer;

			class ConsolePanel {
			public:
				ConsolePanel(ImguiRenderer* imguiRenderer);
				void Render();
				bool AddConsoleMessage(Grindstone::Events::BaseEvent* ev);
			private:
				ImTextureID GetLogSeverityIcon(LogSeverity severity);
			private:
				bool isShowingPanel = true;
				std::deque<ConsoleMessage> messageQueue;

				ImTextureID consoleErrorIcon;
				ImTextureID consoleWarningIcon;
				ImTextureID consoleTraceIcon;
				ImTextureID consoleInfoIcon;
			};
		}
	}
}
