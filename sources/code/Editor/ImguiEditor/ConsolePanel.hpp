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
			class ConsolePanel {
			public:
				ConsolePanel();
				void Render();
				bool AddConsoleMessage(Grindstone::Events::BaseEvent* ev);
			private:
				ImTextureID GetLogSeverityIcon(LogSeverity severity);
				ImTextureID GetTexture(std::string fileName);
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
