#include <EngineCore/Profiling.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>
#include <Editor/EditorManager.hpp>

#include "EditorPluginManager.hpp"
#include "PluginMetaFileLoader.hpp"
#include "PluginManifestFileLoader.hpp"
#include "PluginManifestLockFileLoader.hpp"

using namespace Grindstone::Plugins;
using namespace Grindstone::Utilities;

#include <windows.h>
#include <vector>
#include <thread>

static void ReadPipeLoop(HANDLE pipe, Grindstone::LogSeverity severity) {
	DWORD bytesRead;
	char buffer[4096]{ 0 };
	std::string partial;
	while (ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
		partial.append(buffer, bytesRead);
		size_t pos;
		while ((pos = partial.find('\n')) != std::string::npos) {
			std::string line = partial.substr(0, pos);
			GPRINT(severity, Grindstone::LogSource::Editor, line.c_str());
			partial.erase(0, pos + 1);
		}
	}
	if (!partial.empty()) {
		GPRINT(severity, Grindstone::LogSource::Editor, partial.c_str()); // final line without newline
	}
}

static bool RunCommand(char* commandLine, const char* workingDirectory) {
	SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
	HANDLE stdoutRead, stdoutWrite, stderrRead, stderrWrite;
	CreatePipe(&stdoutRead, &stdoutWrite, &sa, 0);
	SetHandleInformation(stdoutRead, HANDLE_FLAG_INHERIT, 0);
	CreatePipe(&stderrRead, &stderrWrite, &sa, 0);
	SetHandleInformation(stderrRead, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOA si = {};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = stdoutWrite;
	si.hStdError = stderrWrite;
	si.hStdInput = NULL;

	PROCESS_INFORMATION pi = {};
	BOOL success = CreateProcessA(
		NULL,
		static_cast<LPSTR>(commandLine),
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		workingDirectory,
		&si,
		&pi
	);

	CloseHandle(stdoutWrite);
	CloseHandle(stderrWrite);

	if (!success) {
		CloseHandle(stdoutRead);
		CloseHandle(stderrRead);
		return false;
	}

	std::thread readInfoThread(ReadPipeLoop, stdoutRead, Grindstone::LogSeverity::Info);
	std::thread readErrThread(ReadPipeLoop, stderrRead, Grindstone::LogSeverity::Error);

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	readInfoThread.join();
	readErrThread.join();

	return true;
}

static bool RunCMakeCommand(const std::vector<std::string>& cmakeTargets) {
	std::string workingDirectory = EngineCore::GetInstance().GetEngineBinaryPath().parent_path().string();
	
	std::ostringstream cmdStream;
	cmdStream << "cmake";
	cmdStream << " --build .";
	cmdStream << " --config Release";
	cmdStream << " --parallel";
	cmdStream << " --target";
	for (const std::string& target : cmakeTargets) {
		cmdStream << " \"" << target << "\"";
	}
	std::string commandLine = cmdStream.str();
	return RunCommand(commandLine.data(), workingDirectory.empty() ? nullptr : workingDirectory.c_str());
}

static bool RunDotnetCommand(const std::vector<std::string>& dotnetTargets) {
	std::string workingDirectory = (EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins").string();

	std::ostringstream cmdStream;
	cmdStream << "dotnet build ";
	for (const std::string& target : dotnetTargets) {
		cmdStream << " \"" << target.c_str() << "\"";
	}
	std::string commandLine = cmdStream.str();
	return RunCommand(commandLine.data(), workingDirectory.empty() ? nullptr : workingDirectory.c_str());
}

void EditorPluginManager::ResolvePlugins(std::vector<Grindstone::Plugins::ManifestData>& manifestResults) {
	std::filesystem::path basePath = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path();

	std::vector<std::string> cmakeTargetsToCompile;
	std::vector<std::string> dotnetTargetsToCompile;

	for (Grindstone::Plugins::ManifestData& manifestData : manifestResults) {
		Grindstone::Plugins::MetaData metaData{};
		std::filesystem::path metaFilePath = basePath / manifestData.path / "plugin.meta.json";
		if (Grindstone::Plugins::ReadMetaFile(metaFilePath, metaData)) {
			resolvedPluginManifest.emplace_back(metaData);

			for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
				if (!binary.buildTarget.empty()) {
					switch (binary.buildType) {
					case MetaData::BinaryBuildType::NoBuild: {
						std::string path = binary.libraryRelativePath.string();
						GPRINT_ERROR_V(LogSource::Editor, "Binary {} has a target {} but no build type.", path.c_str(), binary.buildTarget.c_str());
						break;
					}
					case MetaData::BinaryBuildType::Cmake:
						cmakeTargetsToCompile.emplace_back(binary.buildTarget);
						break;
					case MetaData::BinaryBuildType::Dotnet:
						dotnetTargetsToCompile.emplace_back(binary.buildTarget);
						break;
					}
				}
				else if (binary.buildType != MetaData::BinaryBuildType::NoBuild) {
					std::string path = binary.libraryRelativePath.string();
					GPRINT_ERROR_V(LogSource::Editor, "Binary {} has a build type other than 'NoBuild' but not a target.", path.c_str());
				}
			}
		}
	}

	if (!cmakeTargetsToCompile.empty()) {
		RunCMakeCommand(cmakeTargetsToCompile);
	}

	if (!dotnetTargetsToCompile.empty()) {
		RunDotnetCommand(dotnetTargetsToCompile);
	}
}

EditorPluginManager::~EditorPluginManager() {
	for (auto it = pluginModules.rbegin(); it != pluginModules.rend(); ++it) {
		auto handle = it->second;
		if (handle) {
			auto releaseModuleFnPtr = (void (*)(Interface*))Modules::GetFunction(handle, "ReleaseModule");

			if (releaseModuleFnPtr) {
				releaseModuleFnPtr(EngineCore::GetInstance().GetPluginInterface());
			}
			else {
				std::string pluginName = it->first.string();
				GPRINT_ERROR_V(LogSource::EngineCore, "Unable to call ReleaseModule in plugin: {}", pluginName.c_str());
			}
		}
	}
}

bool EditorPluginManager::PreprocessPlugins() {
	std::vector<Grindstone::Plugins::ManifestData> manifestResults{};
	if (!Grindstone::Plugins::LoadPluginManifestFile(manifestResults)) {
		return false;
	}

	if (!Grindstone::Plugins::LoadPluginManifestLockFile(manifestResults)) {
		return false;
	}

	ResolvePlugins(manifestResults);

	return true;
}

void EditorPluginManager::LoadPluginsByStage(std::string_view stageName) {
	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	std::filesystem::path basePath = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins";

	for (Grindstone::Plugins::MetaData& metaData : resolvedPluginManifest) {
		std::filesystem::path pluginPath = basePath / metaData.name;
		for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
			if (binary.loadStage == stageName) {
				std::filesystem::path binaryPath = pluginPath / binary.libraryRelativePath;
				std::filesystem::path parentPath = binaryPath.parent_path();
				DLL_DIRECTORY_COOKIE dllCookie = AddDllDirectory(parentPath.wstring().c_str());
				LoadModule(binaryPath);
				RemoveDllDirectory(dllCookie);
			}
		}

		for (Grindstone::Plugins::MetaData::AssetDirectory& assetDir : metaData.assetDirectories) {
			if (assetDir.loadStage == stageName) {
				std::filesystem::path assetsPath = pluginPath / assetDir.assetDirectoryRelativePath;
				Editor::FileManager& fileManager = editorManager.GetFileManager();
				fileManager.MountDirectory(assetDir.mountPoint, assetsPath);
			}
		}
	}
}

void EditorPluginManager::UnloadPluginsByStage(std::string_view stageName) {
	Editor::Manager& editorManager = Editor::Manager::GetInstance();
	std::filesystem::path basePath = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins";

	for (Grindstone::Plugins::MetaData& metaData : resolvedPluginManifest) {
		std::filesystem::path pluginPath = basePath / metaData.name;
		for (Grindstone::Plugins::MetaData::AssetDirectory& assetDir : metaData.assetDirectories) {
			if (assetDir.loadStage == stageName) {
				std::filesystem::path assetsPath = pluginPath / assetDir.assetDirectoryRelativePath;
				Editor::FileManager& fileManager = editorManager.GetFileManager();
				fileManager.UnmountDirectory(assetDir.mountPoint);
			}
		}

		for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
			if (binary.loadStage == stageName) {
				std::filesystem::path binaryPath = pluginPath / binary.libraryRelativePath;
				UnloadModule(binaryPath);
			}
		}
	}
}

std::filesystem::path Grindstone::Plugins::EditorPluginManager::GetLibraryPath(std::string_view pluginName, std::string_view libraryName) {
	std::filesystem::path basePath = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins";

	for (Grindstone::Plugins::MetaData& metaData : resolvedPluginManifest) {
		if (metaData.name == pluginName) {
			for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
				std::filesystem::path filename = binary.libraryRelativePath.filename();
				filename.replace_extension();
				if (filename.string() == libraryName) {
					filename = basePath / metaData.name / binary.libraryRelativePath;
					filename.replace_extension("dll");
					return filename;
				}
			}

			return "";
		}
	}

	return "";
}

bool EditorPluginManager::LoadModule(const std::filesystem::path& path) {
	auto it = pluginModules.find(path);
	if (it != pluginModules.end()) {
#ifdef _DEBUG
		throw std::runtime_error("Module already loaded! This error only exists in debug mode to make sure you don't write useless code.");
#endif
		return true;
	}

	SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
	auto handle = Modules::Load(path.string().c_str());

	if (handle) {
		pluginModules[path] = handle;

		auto initializeModuleFnPtr = (void (*)(Interface*))Modules::GetFunction(handle, "InitializeModule");

		if (initializeModuleFnPtr) {
			initializeModuleFnPtr(EngineCore::GetInstance().GetPluginInterface());

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

void EditorPluginManager::UnloadModule(const std::filesystem::path& path) {
	auto it = pluginModules.find(path);
	if (it == pluginModules.end()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Unable to unload plugin \"{0}\"", path.string());
		return;
	}

	Grindstone::Utilities::Modules::Handle handle = it->second;
	if (handle) {
		auto releaseModuleFnPtr = static_cast<void (*)(Interface*)>(Modules::GetFunction(handle, "ReleaseModule"));

		if (releaseModuleFnPtr) {
			releaseModuleFnPtr(EngineCore::GetInstance().GetPluginInterface());
		}
		else {
			GPRINT_ERROR_V(LogSource::EngineCore, "Unable to call ReleaseModule in plugin: {0}", it->first.string().c_str());
		}

		Grindstone::Utilities::Modules::Unload(handle);
	}

	pluginModules.erase(it);
}

void EditorPluginManager::QueueInstall(std::string pluginName) {
	queuedInstalls.insert(pluginName);

	if (queuedUninstalls.contains(pluginName)) {
		queuedUninstalls.erase(pluginName);
	}
}

void EditorPluginManager::QueueUninstall(std::string pluginName) {
	queuedUninstalls.insert(pluginName);

	if (queuedInstalls.contains(pluginName)) {
		queuedInstalls.erase(pluginName);
	}
}

void EditorPluginManager::ProcessQueuedPluginInstallsAndUninstalls() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	std::filesystem::path basePath = engineCore.GetEngineBinaryPath().parent_path() / "plugins";

	if (!queuedUninstalls.empty()) {
		engineCore.GetGraphicsCore()->WaitUntilIdle();

		for (const std::string& plugin : queuedUninstalls) {
			for (size_t metaDataIndex = 0; metaDataIndex < resolvedPluginManifest.size(); ++metaDataIndex) {
				Grindstone::Plugins::MetaData& metaData = resolvedPluginManifest[metaDataIndex];
				if (metaData.name == plugin) {
					std::filesystem::path pluginPath = basePath / metaData.name;
					for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
						std::filesystem::path binaryPath = pluginPath / binary.libraryRelativePath;
						UnloadModule(binaryPath);
					}

					resolvedPluginManifest.erase(resolvedPluginManifest.begin() + metaDataIndex);
					break;
				}
			}
		}
		queuedUninstalls.clear();
	}

	if (!queuedInstalls.empty()) {
		std::filesystem::path basePluginsRelativePath("plugins");
		std::vector<ManifestData> pluginsToInstall;
		for (const std::string& plugin : queuedInstalls) {
			pluginsToInstall.emplace_back(
				ManifestData{
					.pluginName = plugin.c_str(),
					.semanticVersioning = ">0.0.1",
					.path = basePluginsRelativePath / plugin
				}
			);
		}
		ResolvePlugins(pluginsToInstall);

		for (Grindstone::Plugins::MetaData& metaData : resolvedPluginManifest) {
			bool toInstall = false;
			for (const std::string& pluginToInstall : queuedInstalls) {
				if (metaData.name == pluginToInstall) {
					toInstall = true;
				}
				break;
			}

			if (!toInstall) {
				continue;
			}

			std::filesystem::path pluginPath = basePath / metaData.name;

			for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
				std::filesystem::path binaryPath = pluginPath / binary.libraryRelativePath;
				std::filesystem::path parentPath = binaryPath.parent_path();
				DLL_DIRECTORY_COOKIE dllCookie = AddDllDirectory(parentPath.wstring().c_str());
				LoadModule(binaryPath);
				RemoveDllDirectory(dllCookie);
			}
		}

		queuedInstalls.clear();
	}
}

EditorPluginManager::Iterator EditorPluginManager::begin() noexcept {
	return resolvedPluginManifest.begin();
}

EditorPluginManager::ConstIterator EditorPluginManager::begin() const noexcept {
	return resolvedPluginManifest.begin();
}

EditorPluginManager::Iterator EditorPluginManager::end() noexcept {
	return resolvedPluginManifest.end();
}

EditorPluginManager::ConstIterator EditorPluginManager::end() const noexcept {
	return resolvedPluginManifest.end();
}
