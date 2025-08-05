#include <EngineCore/Profiling.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>

#include "Manager.hpp"
#include "Interface.hpp"
#include "PluginManifestFileLoader.hpp"
#include "PluginManifestLockFileLoader.hpp"

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
				std::string pluginName = it->first.string();
				GPRINT_ERROR_V(LogSource::EngineCore, "Unable to call ReleaseModule in plugin: {}", pluginName.c_str());
			}
		}
	}
}

#include <EngineCore/PluginSystem/PluginMetaFileLoader.hpp>

bool Manager::LoadPluginList() {
	std::vector<Grindstone::Plugins::ManifestData> manifestResults{};
	if (!Grindstone::Plugins::LoadPluginManifestFile(manifestResults)) {
		return false;
	}

	if (!Grindstone::Plugins::LoadPluginManifestLockFile(manifestResults)) {
		return false;
	}

	std::filesystem::path basePath = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path();

	for (Grindstone::Plugins::ManifestData& manifestData : manifestResults) {
		Grindstone::Plugins::MetaData metaData{};
		std::filesystem::path metaFilePath = basePath / manifestData.path / "plugin.meta.json";
		if (Grindstone::Plugins::ReadMetaFile(metaFilePath, metaData)) {
			resolvedPluginManifest.emplace_back(metaData);
		}
	}

	return true;
}

void Manager::LoadPluginsOfStage(const char* stageName) {
	std::filesystem::path basePath = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins";

	for (Grindstone::Plugins::MetaData& metaData : resolvedPluginManifest) {
		if (metaData.loadStage != stageName) {
			continue;
		}

		std::filesystem::path pluginPath = basePath / metaData.name;
		for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
			std::filesystem::path binaryPath = pluginPath / binary.libraryRelativePath;
			Load(binaryPath.string().c_str());
		}
	}
}

void Manager::UnloadPluginListExceptRenderHardwareInterface() {
	for (int32_t i = static_cast<int32_t>(pluginsFromList.size()) - 1; i > 0; --i) {
		Grindstone::Utilities::Modules::Handle handle = pluginsFromList[i];
		if (handle) {
			auto releaseModuleFnPtr = (void (*)(Interface*))Modules::GetFunction(handle, "ReleaseModule");

			// Get the name and erase the element from the map.
			std::string name;
			for (auto& mapElement : plugins) {
				if (mapElement.second == handle) {
					name = mapElement.first.string();
					plugins.erase(name);
					break;
				}
			}

			if (releaseModuleFnPtr) {
				releaseModuleFnPtr(&pluginInterface);
			}
			else {
				GPRINT_ERROR_V(LogSource::EngineCore, "Unable to call ReleaseModule in plugin: {0}", name.c_str());
			}
		}
	}
}

void Manager::UnloadPluginRenderHardwareInterface() {
	Grindstone::Utilities::Modules::Handle handle = pluginsFromList[0];
	if (handle) {
		auto releaseModuleFnPtr = (void (*)(Interface*))Modules::GetFunction(handle, "ReleaseModule");

		// Get the name and erase the element from the map.
		std::string pluginName = plugins.begin()->first.string();
		plugins.erase(plugins.begin());

		if (releaseModuleFnPtr) {
			releaseModuleFnPtr(&pluginInterface);
		}
		else {
			GPRINT_ERROR_V(LogSource::EngineCore, "Unable to call ReleaseModule in plugin: {0}", pluginName.c_str());
		}
	}
}

void Manager::SetupInterfacePointers() {
	const EngineCore& engineCore = EngineCore::GetInstance();
	pluginInterface.systemRegistrar = engineCore.GetSystemRegistrar();
	pluginInterface.componentRegistrar = engineCore.GetComponentRegistrar();
}

Interface& Manager::GetInterface() {
	return pluginInterface;
}

bool Manager::Load(const std::filesystem::path& path) {
#ifdef _DEBUG
	std::string profileStr = std::string("Loading module ") + path.string();
	GRIND_PROFILE_SCOPE(profileStr.c_str());
#endif

	// Return true if plugin already loaded
	auto it = plugins.find(path);
	if (it != plugins.end()) {
#ifdef _DEBUG
		throw std::runtime_error("Module already loaded! This error only exists in debug mode to make sure you don't write useless code.");
#endif
		return true;
	}

	std::wstring parentDirectory = path.parent_path().wstring();
	AddDllDirectory(parentDirectory.c_str());
	SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
	auto handle = Modules::Load(path.string().c_str());

	if (handle) {
		plugins[path] = handle;
		pluginsFromList.emplace_back(handle);

		auto initializeModuleFnPtr = (void (*)(Interface*))Modules::GetFunction(handle, "InitializeModule");

		if (initializeModuleFnPtr) {
			initializeModuleFnPtr(&pluginInterface);

			return true;
		}
		else {
			GPRINT_ERROR_V(LogSource::EngineCore, "Unable to call InitializeModule in plugin: {0}", path.string().c_str());
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

	GPRINT_ERROR_V(LogSource::EngineCore, "Unable to load plugin \"{0}\": {1}", path.string(), errorString);
#else
	GPRINT_ERROR_V(LogSource::EngineCore, "Unable to load plugin: {0}", path.string());
#endif

	return false;
}

void Manager::LoadCritical(const std::filesystem::path& path) {
	if (!Load(path)) {
		throw std::runtime_error(std::string("Failed to load module: ") + path.string());
	}
}

void Manager::Remove(const std::filesystem::path& path) {
	auto it = plugins.find(path);
	if (it != plugins.end()) {
		Grindstone::Utilities::Modules::Handle handle = it->second;
		if (handle) {
			auto releaseModuleFnPtr = static_cast<void (*)(Interface*)>(Modules::GetFunction(handle, "ReleaseModule"));

			if (releaseModuleFnPtr) {
				releaseModuleFnPtr(&pluginInterface);
			}
			else {
				GPRINT_ERROR_V(LogSource::EngineCore, "Unable to call ReleaseModule in plugin: {0}", it->first.string().c_str());
			}
		}

		for (size_t i = 0; i < pluginsFromList.size(); ++i) {
			if (pluginsFromList[i] == handle) {
				pluginsFromList.erase(pluginsFromList.begin() + i);
				break;
			}
		}

		plugins.erase(it);
	}
}
