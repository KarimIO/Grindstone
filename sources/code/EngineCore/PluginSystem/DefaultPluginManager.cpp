#include <Common/Logger.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Profiling.hpp>
#include <EngineCore/Utils/Utilities.hpp>

#include <rapidjson/document.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../../../../plugins/Grindstone.Script.CSharp/include/CSharpManager.hpp"

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
				GPRINT_ERROR(LogSource::PluginSystem, "Unable to call ReleaseModule in plugin: {}", pluginName.c_str());
			}
		}
	}

	pluginModules.clear();
}

bool DefaultPluginManager::PreprocessPlugins() {
	EngineCore& engineCore = EngineCore::GetInstance();
	std::filesystem::path pluginsManifest = engineCore.GetProjectPath() / "buildSettings" / "pluginsManifest.txt";
	std::string pluginsManifestStr = pluginsManifest.string();
	if (!std::filesystem::exists(pluginsManifest)) {
		GPRINT_ERROR(LogSource::PluginSystem, "Failed to find plugins manifest: {}", pluginsManifestStr.c_str());
		return false;
	}

	std::string contents = Grindstone::Utils::LoadFileText(pluginsManifestStr.c_str());

	rapidjson::Document document;
	rapidjson::ParseResult parseResult = document.Parse(contents.c_str());
	if (parseResult.IsError()) {
		rapidjson::GetParseErrorFunc GetParseError = rapidjson::GetParseErrorFunc();
		const RAPIDJSON_ERROR_CHARTYPE* errorCode = "Unknown Error";
		if (GetParseError != nullptr) {
			errorCode = GetParseError(parseResult.Code());
		}
		GPRINT_ERROR(LogSource::PluginSystem,
					"Failed to parse plugins manifest: '{}' {} ({})",
					pluginsManifestStr.c_str(),
					errorCode,
					document.GetErrorOffset());
		return false;
	}

	GS_ASSERT_ENGINE_WITH_MESSAGE(document.IsObject(), "Plugin Manifest document does not start with an object.");

	for (auto loadStageIterator = document.MemberBegin(); loadStageIterator != document.MemberEnd(); loadStageIterator++) {
		GS_ASSERT_ENGINE(loadStageIterator->name.IsString());
		GS_ASSERT_ENGINE(loadStageIterator->value.IsArray());

		const char* loadStage = loadStageIterator->name.GetString();
		rapidjson::GenericArray loadStageBinaries = loadStageIterator->value.GetArray();

		LoadStageList& pluginsForLoadStage = pluginsMapByLoadStage[loadStage];

		for (size_t i = 0; i < loadStageBinaries.Size(); i++) {
			rapidjson::Value& entry = loadStageBinaries[i];
			GS_ASSERT_ENGINE(entry.IsObject());
			GS_ASSERT_ENGINE(entry.HasMember("identifier"));
			GS_ASSERT_ENGINE(entry.HasMember("filename"));
			GS_ASSERT_ENGINE(entry.HasMember("binaryType"));

			rapidjson::Value& identifierJsonValue = entry["identifier"];
			rapidjson::Value& filenameJsonValue = entry["filename"];
			rapidjson::Value& binaryTypeJsonValue = entry["binaryType"];
			GS_ASSERT_ENGINE(identifierJsonValue.IsString());
			GS_ASSERT_ENGINE(filenameJsonValue.IsString());
			GS_ASSERT_ENGINE(binaryTypeJsonValue.IsString());

			const char* identifierStr = identifierJsonValue.GetString();
			const char* filenameStr = filenameJsonValue.GetString();
			const char* binaryTypeStr = binaryTypeJsonValue.GetString();

			BinaryType binaryType = BinaryType::NoBuild;
			if (strcmp(binaryTypeStr, "cpp") == 0) {
				binaryType = BinaryType::Cpp;
			}
			else if (strcmp(binaryTypeStr, "dotnet") == 0) {
				binaryType = BinaryType::Dotnet;
			}

			pluginsForLoadStage.emplace_back(
				BinaryEntry{
					.binaryIdentifier = identifierStr,
					.binaryFilename = filenameStr,
					.buildType = binaryType
				}
			);
		}
	}

	return true;
}

void DefaultPluginManager::LoadPluginsByStage(std::string_view stageName) {
	auto loadStageIterator = pluginsMapByLoadStage.find(std::string(stageName));
	if (loadStageIterator == pluginsMapByLoadStage.end()) {
		return;
	}

	const LoadStageList& pluginsForLoadStage = loadStageIterator->second;
	for (const BinaryEntry& entry : pluginsForLoadStage) {
		if (entry.buildType == BinaryType::Cpp) {
			GPRINT_INFO(LogSource::Editor, "[PluginSystem] In stage {}, mounting C++ (cmake) plugin: {}", stageName, entry.binaryIdentifier);
			LoadModule(entry.binaryFilename);
		}
		else {
			auto scriptManager = Grindstone::EngineCore::GetInstance().TryGetService<Grindstone::Scripting::CSharp::CSharpManager>();
			GPRINT_INFO(LogSource::Editor, "[PluginSystem] In stage {}, mounting C# (dotnet) plugin: {}", stageName, entry.binaryIdentifier);
			scriptManager->LoadAssemblyIntoMap(entry.binaryIdentifier);
			break;
		}
	}
}

void DefaultPluginManager::UnloadPluginsByStage(std::string_view stageName) {
	auto loadStageIterator = pluginsMapByLoadStage.find(std::string(stageName));
	if (loadStageIterator == pluginsMapByLoadStage.end()) {
		return;
	}

	const LoadStageList& pluginsForLoadStage = loadStageIterator->second;
	for (const BinaryEntry& entry : pluginsForLoadStage) {
		UnloadModule(entry.binaryFilename);
	}
}

std::filesystem::path Grindstone::Plugins::DefaultPluginManager::GetLibraryPath(std::string_view pluginName, std::string_view libraryName) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();

	std::string searchBinaryName = std::string(pluginName) + std::string(":") + std::string(libraryName);
	for (const auto& loadStageIterator : pluginsMapByLoadStage) {
		const std::string& pluginName = loadStageIterator.first;
		const LoadStageList& pluginsForLoadStage = loadStageIterator.second;
		for (const BinaryEntry& entry : pluginsForLoadStage) {
			if (entry.binaryIdentifier == searchBinaryName) {
				return engineCore.GetBinaryPath() / CMAKE_INTDIR /  (entry.binaryFilename + ".dll");
			}
		}
	}

	return "";
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

	if (!std::filesystem::exists(path + ".dll")) {
		GPRINT_ERROR(LogSource::PluginSystem, "Unable to load plugin \"{0}\": DLL not found at path", path.c_str());
		return false;
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
			GPRINT_ERROR(LogSource::PluginSystem, "Unable to call InitializeModule in plugin: {0}", path.c_str());
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

	GPRINT_ERROR(LogSource::PluginSystem, "Unable to load plugin \"{0}\": {1}", path.c_str(), errorString);
#else
	GPRINT_ERROR(LogSource::PluginSystem, "Unable to load plugin: {0}", path.c_str());
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
				GPRINT_ERROR(LogSource::PluginSystem, "Unable to call ReleaseModule in plugin: {0}", it->first.c_str());
			}

			Grindstone::Utilities::Modules::Unload(handle);
		}

		pluginModules.erase(it);
	}
}
