#include <EngineCore/Profiling.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>

#include "DefaultPluginManager.hpp"
#include "Interface.hpp"

using namespace Grindstone::Plugins;
using namespace Grindstone::Utilities;

DefaultPluginManager::~DefaultPluginManager() {
	for (auto it = pluginModules.rbegin(); it != pluginModules.rend(); ++it) {
		auto handle = it->second;
		if (handle) {
			auto releaseModuleFnPtr = (void (*)(Interface*))Modules::GetFunction(handle, "ReleaseModule");

			if (releaseModuleFnPtr) {
				releaseModuleFnPtr(EngineCore::GetInstance().GetPluginInterface());
			}
			else {
				const std::string& pluginName = it->first;
				GPRINT_ERROR_V(LogSource::EngineCore, "Unable to call ReleaseModule in plugin: {}", pluginName.c_str());
			}
		}
	}

	pluginModules.clear();
}

bool DefaultPluginManager::PreprocessPlugins() {
	return true;
}

void DefaultPluginManager::LoadPluginsByStage(const char* stageName) {
	std::filesystem::path basePath = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins";
}

void DefaultPluginManager::UnloadPluginsByStage(const char* stageName) {
	std::filesystem::path basePath = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins";
}

bool DefaultPluginManager::LoadModule(const std::string& path) {
#ifdef _DEBUG
	std::string profileStr = std::string("Loading module ") + path;
	GRIND_PROFILE_SCOPE(profileStr.c_str());
#endif

	// Return true if plugin already loaded
	auto it = pluginModules.find(path);
	if (it != pluginModules.end()) {
#ifdef _DEBUG
		throw std::runtime_error("Module already loaded! This error only exists in debug mode to make sure you don't write useless code.");
#endif
		return true;
	}

	SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
	auto handle = Modules::Load(path.c_str());

	if (handle) {
		pluginModules[path] = handle;

		auto initializeModuleFnPtr = (void (*)(Interface*))Modules::GetFunction(handle, "InitializeModule");

		if (initializeModuleFnPtr) {
			initializeModuleFnPtr(EngineCore::GetInstance().GetPluginInterface());

			return true;
		}
		else {
			GPRINT_ERROR_V(LogSource::EngineCore, "Unable to call InitializeModule in plugin: {0}", path.c_str());
			return false;
		}
	}

#ifdef _WIN32
	const DWORD lastError = GetLastError();
	char errorString[256] = {};
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr,
		lastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		errorString,
		255,
		nullptr
	);

	GPRINT_ERROR_V(LogSource::EngineCore, "Unable to load plugin \"{0}\": {1}", path.c_str(), errorString);
#else
	GPRINT_ERROR_V(LogSource::EngineCore, "Unable to load plugin: {0}", path.c_str());
#endif

	return false;
}

void DefaultPluginManager::UnloadModule(const std::string& path) {
	auto it = pluginModules.find(path);
	if (it != pluginModules.end()) {
		Grindstone::Utilities::Modules::Handle handle = it->second;
		if (handle) {
			auto releaseModuleFnPtr = static_cast<void (*)(Interface*)>(Modules::GetFunction(handle, "ReleaseModule"));

			if (releaseModuleFnPtr) {
				releaseModuleFnPtr(EngineCore::GetInstance().GetPluginInterface());
			}
			else {
				GPRINT_ERROR_V(LogSource::EngineCore, "Unable to call ReleaseModule in plugin: {0}", it->first.c_str());
			}

			Grindstone::Utilities::Modules::Unload(handle);
		}

		pluginModules.erase(it);
	}
}
