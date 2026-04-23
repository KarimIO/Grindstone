#pragma once

#include <filesystem>

#include "Editor/BuildProcess.hpp"

namespace Grindstone::Assets::AssetPackSerializer {
	void SerializeAllAssets(const std::filesystem::path& targetPath, Editor::BuildProcessStats* buildProgress, float minProgress, float deltaProgress);
}
