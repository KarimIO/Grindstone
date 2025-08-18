#include <EngineCore/Profiling.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>

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
	char buffer[4096];
	std::string partial;
	while (ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
		partial.append(buffer, bytesRead);
		size_t pos;
		while ((pos = partial.find('\n')) != std::string::npos) {
			std::string line = partial.substr(0, pos);
			GPRINT(severity, Grindstone::LogSource::EngineCore, line.c_str());
			partial.erase(0, pos + 1);
		}
	}
	if (!partial.empty()) {
		GPRINT(severity, Grindstone::LogSource::EngineCore, partial.c_str()); // final line without newline
	}
}

static bool RunCMakeCommand(
	const std::vector<std::string>& cmakeTargets
) {
	std::string workingDirectory = EngineCore::GetInstance().GetEngineBinaryPath().parent_path().string();
	std::vector<std::string> args = { "--build", ".", "--target" };

	for (const std::string& target : cmakeTargets) {
		args.emplace_back(target);
	}

	args.emplace_back("--config");
	args.emplace_back("Release");

	std::ostringstream cmdStream;
	cmdStream << "cmake";
	for (const auto& arg : args) {
		cmdStream << " \"" << arg << "\"";
	}
	std::string commandLine = cmdStream.str();

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
		commandLine.data(),
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		workingDirectory.empty() ? NULL : workingDirectory.c_str(),
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

	std::filesystem::path basePath = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path();

	std::vector<std::string> cmakeTargetsToCompile;

	for (Grindstone::Plugins::ManifestData& manifestData : manifestResults) {
		Grindstone::Plugins::MetaData metaData{};
		std::filesystem::path metaFilePath = basePath / manifestData.path / "plugin.meta.json";
		if (Grindstone::Plugins::ReadMetaFile(metaFilePath, metaData)) {
			resolvedPluginManifest.emplace_back(metaData);

			for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
				if (!binary.cmakeTarget.empty()) {
					cmakeTargetsToCompile.emplace_back(binary.cmakeTarget);
				}
			}
		}
	}

	if (!cmakeTargetsToCompile.empty()) {
		RunCMakeCommand(cmakeTargetsToCompile);
	}

	return true;
}

void EditorPluginManager::LoadPluginsByStage(const char* stageName) {
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
	}
}

void EditorPluginManager::UnloadPluginsByStage(const char* stageName) {
	std::filesystem::path basePath = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins";

	for (Grindstone::Plugins::MetaData& metaData : resolvedPluginManifest) {
		std::filesystem::path pluginPath = basePath / metaData.name;
		for (Grindstone::Plugins::MetaData::Binary& binary : metaData.binaries) {
			if (binary.loadStage == stageName) {
				std::filesystem::path binaryPath = pluginPath / binary.libraryRelativePath;
				UnloadModule(binaryPath);
			}
		}
	}
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
