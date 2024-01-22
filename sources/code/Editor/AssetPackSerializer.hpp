#pragma once

#include <atomic>

namespace Grindstone::Assets::AssetPackSerializer {
	void SerializeAllAssets(std::atomic<float>* outStageProgress, std::atomic<float>* outProgress);
}
