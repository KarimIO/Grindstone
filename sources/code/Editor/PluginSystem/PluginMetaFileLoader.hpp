#pragma once

#include <filesystem>
#include "PluginMetaData.hpp"

namespace Grindstone::Plugins {
	bool ReadMetaFile(std::filesystem::path path, Grindstone::Plugins::MetaData& metaData);
}
