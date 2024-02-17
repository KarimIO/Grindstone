#include "Manager.hpp"
#include "Interface.hpp"
#include "../Profiling.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Logger.hpp"
using namespace Grindstone::Plugins;
using namespace Grindstone::Utilities;

Manager::Manager(EngineCore* engineCore) : pluginInterface(this), engineCore(engineCore) {
	SetupInterfacePointers();
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

void Manager::LoadPluginList() {
	auto& engineCore = EngineCore::GetInstance();
	std::filesystem::path pluginListFIle = engineCore.GetProjectPath() / "buildSettings/pluginsManifest.txt";
	auto prefabListFilePath = pluginListFIle.string();
	auto fileContents = Utils::LoadFileText(prefabListFilePath.c_str());

	size_t start = 0, end;
	std::string pluginName;
	while (true) {
		end = fileContents.find("\n", start);
		if (end == std::string::npos) {
			pluginName = fileContents.substr(start);
			if (!pluginName.empty()) {
				Load(pluginName.c_str());
			}

			break;
		}

		pluginName = fileContents.substr(start, end - start);
		Load(pluginName.c_str());
		start = end + 1;
	}
}

void Manager::SetupInterfacePointers() {
	const EngineCore& engineCore = EngineCore::GetInstance();
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

	Logger::Print(LogSeverity::Error, "Unable to load plugin \"{0}\": {1}", path, errorString);
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
			auto releaseModuleFnPtr = static_cast<void (*)(Interface*)>(Modules::GetFunction(handle, "ReleaseModule"));

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
