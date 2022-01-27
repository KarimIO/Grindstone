#include "Manager.hpp"
#include "Interface.hpp"
#include "../Profiling.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Logger.hpp"
using namespace Grindstone::Plugins;
using namespace Grindstone::Utilities;

Manager::Manager(EngineCore* engineCore) : pluginInterface(this), engineCore(engineCore) {
}

Manager::~Manager() {
	for (auto it = plugins.rbegin(); it != plugins.rend(); ++it) {
		auto handle = it->second;
		if (handle) {
			auto releaseModuleFnPtr = (void (*)(Interface*))Modules::GetFunction(handle, "ReleaseModule");

			if (releaseModuleFnPtr) {
				releaseModuleFnPtr(&pluginInterface);
			}
			else {
				Logger::Print(LogSeverity::Error, "Unable to call ReleaseModule in plugin: {0}", it->first.c_str());
			}
		}
	}
}

void Manager::SetupManagers() {
	auto& engineCore = EngineCore::GetInstance();
	pluginInterface.systemRegistrar = engineCore.GetSystemRegistrar();
	pluginInterface.componentRegistrar = engineCore.GetComponentRegistrar();
}

bool Manager::Load(const char *path) {
#ifdef _DEBUG
	std::string profile_str = std::string("Loading module ") + path;
	GRIND_PROFILE_SCOPE(profile_str.c_str());
#endif

	// Return true if plugin already loaded
	auto it = plugins.find(path);
	if (it != plugins.end()) {
#ifdef _DEBUG
		throw std::runtime_error("Module already loaded! This error only exists in debug mode to make sure you don't write useless code.");
#endif
		return true;
	}

	auto handle = Modules::Load(path);

	if (handle) {
		plugins[path] = handle;

		auto initializeModuleFnPtr = (void (*)(Interface*))Modules::GetFunction(handle, "InitializeModule");

		if (initializeModuleFnPtr) {
			initializeModuleFnPtr(&pluginInterface);

			return true;
		}
		else {
			Logger::Print(LogSeverity::Error, "Unable to call InitializeModule in plugin: {0}", path);
			return false;
		}
	}

#ifdef _WIN32
	DWORD lastError = GetLastError();
	Logger::Print(LogSeverity::Error, "Unable to load plugin {0} with {1}", path, lastError);
#else
	Logger::Print(LogSeverity::Error, "Unable to load plugin: {0}", path);
#endif
	return false;
}

void Manager::LoadCritical(const char* path) {
	if (!Load(path)) {
		throw std::runtime_error(std::string("Failed to load module: ") + path);
	}
}

void Manager::Remove(const char* name) {
	auto it = plugins.find(name);
	if (it != plugins.end()) {
		auto handle = it->second;
		if (handle) {
			auto releaseModuleFnPtr = (void (*)(Interface*))Modules::GetFunction(handle, "ReleaseModule");

			if (releaseModuleFnPtr) {
				releaseModuleFnPtr(&pluginInterface);
			}
			else {
				Logger::Print(LogSeverity::Error, "Unable to call ReleaseModule in plugin: {0}", it->first.c_str());
			}
		}

		plugins.erase(it);
	}
}
