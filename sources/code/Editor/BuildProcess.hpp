#pragma once

#include <atomic>
#include <string>
#include <mutex>

namespace Grindstone::Editor {
	struct BuildProcessStats {
		std::atomic<float> progress;
		std::string stageText;
		std::string detailText;
		std::mutex stringMutex;
	};

	void BuildGame(BuildProcessStats* buildProcessStats);
}
