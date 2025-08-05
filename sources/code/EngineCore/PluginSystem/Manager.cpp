#include <EngineCore/Profiling.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/PluginSystem/PluginMetaFileLoader.hpp>

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
	for (auto it = pluginBinaries.rbegin(); it != pluginBinaries.rend(); ++it) {
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

void Manager::LoadPluginBinariesAndAssetsOfStage(const char* stageName) {
	std::filesystem::path basePath = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins";

	for (Grindstone::Plugins::MetaData& metaData : resolvedPluginManifest) {
		std::filesystem::path pluginPath = basePath / metaData.name;
		for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
			if (binary.loadStage == stageName) {
				std::filesystem::path binaryPath = pluginPath / binary.libraryRelativePath;
				std::filesystem::path parentPath = binaryPath.parent_path();
				DLL_DIRECTORY_COOKIE dllCookie = AddDllDirectory(parentPath.wstring().c_str());
				Load(binaryPath);
				RemoveDllDirectory(dllCookie);
			}
		}
	}
}

void Manager::UnloadPluginBinariesAndAssetsFromStage(const char* stageName) {
	std::filesystem::path basePath = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins";

	for (Grindstone::Plugins::MetaData& metaData : resolvedPluginManifest) {
		std::filesystem::path pluginPath = basePath / metaData.name;
		for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
			if (binary.loadStage == stageName) {
				std::filesystem::path binaryPath = pluginPath / binary.libraryRelativePath;
				Remove(binaryPath);
			}
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
	auto it = pluginBinaries.find(path);
	if (it != pluginBinaries.end()) {
#ifdef _DEBUG
		throw std::runtime_error("Module already loaded! This error only exists in debug mode to make sure you don't write useless code.");
#endif
		return true;
	}

	SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
	auto handle = Modules::Load(path.string().c_str());

	if (handle) {
		pluginBinaries[path] = handle;

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
	auto it = pluginBinaries.find(path);
	if (it != pluginBinaries.end()) {
		Grindstone::Utilities::Modules::Handle handle = it->second;
		if (handle) {
			auto releaseModuleFnPtr = static_cast<void (*)(Interface*)>(Modules::GetFunction(handle, "ReleaseModule"));

			if (releaseModuleFnPtr) {
				releaseModuleFnPtr(&pluginInterface);
			}
			else {
				GPRINT_ERROR_V(LogSource::EngineCore, "Unable to call ReleaseModule in plugin: {0}", it->first.string().c_str());
			}

			Grindstone::Utilities::Modules::Unload(handle);
		}

		pluginBinaries.erase(it);
	}
}
