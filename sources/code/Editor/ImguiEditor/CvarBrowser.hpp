#pragma once

#include <vector>

#include <Common/Console/Cvars.hpp>

namespace Grindstone::Editor::ImguiEditor {
	class CvarBrowser {
	public:
		CvarBrowser();
		void Render();
	private:
		bool isShowingPanel = false;
		std::vector<Grindstone::CvarParameter*> cachedEditParameters;
	};
}
