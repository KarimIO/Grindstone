#pragma once

#include <thread>
#include <Editor/BuildProcess.hpp>

namespace Grindstone::Editor::ImguiEditor {
	class BuildPopup {
	public:
		void StartBuild();
		void Render();

	private:

		bool isCompilingAssets;
		Grindstone::Editor::BuildProcessStats processStatus;
		std::thread buildThread;
	};
}
