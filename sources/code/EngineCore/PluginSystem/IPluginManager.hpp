#pragma once

#include <filesystem>

namespace Grindstone::Plugins {
	class Interface;

	class IPluginManager {
	public:
		virtual ~IPluginManager() {}

		virtual bool PreprocessPlugins() = 0;
		virtual void LoadPluginsByStage(const char* stageName) = 0;
		virtual void UnloadPluginsByStage(const char* stageName) = 0;
	};
}
