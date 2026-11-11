#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <string>

namespace Grindstone::Editor {
	struct BuildProcessStats {
		std::atomic<float> progress;
		std::string stageText;
		std::string detailText;
		std::mutex stringMutex;
	};

	void BuildGame(BuildProcessStats* buildProcessStats, std::function<void()> onCancel);
}
