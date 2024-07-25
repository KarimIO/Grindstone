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

				struct EditorConsoleMessage {
					ConsoleMessage base;
					std::string lowercaseMessage;
					uint32_t count;

					EditorConsoleMessage(
						ConsoleMessage base,
						std::string lowercaseMessage,
						uint32_t count
					) : base(base), lowercaseMessage(lowercaseMessage), count(count) {};
				};
			private:
				void RenderTopbar();
				void RenderMessage(size_t index, EditorConsoleMessage& msg);
				void FilterSearch();
				void RenderButton(const char* title, ImTextureID icon, uint8_t severityBit);
				ImTextureID GetLogSeverityIcon(LogSeverity severity) const;
			private:
				bool isShowingPanel = true;
				std::deque<EditorConsoleMessage> messageQueue;
				std::vector<EditorConsoleMessage*> filteredMessage;
				std::string filterText;
				std::string filterTextLowercase;
				uint8_t severityFlags;
				uint64_t sourceFlags;

				ImVec4 selectedColor;
				ImVec4 selectedHighlightColor;
				ImVec4 selectedActiveColor;

				ImTextureID consoleErrorIcon;
				ImTextureID consoleWarningIcon;
				ImTextureID consoleTraceIcon;
				ImTextureID consoleInfoIcon;
			};
		}
	}
}
