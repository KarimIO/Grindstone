#pragma once

#include "Editor/BuildProcess.hpp"

namespace Grindstone::Assets::AssetPackSerializer {
	void SerializeAllAssets(std::filesystem::path targetPath, Editor::BuildProcessStats* buildProgress, float minProgress, float deltaProgress);
}
