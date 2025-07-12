#include <fstream>
#include <regex>

#ifdef _MSC_VER
#include <Windows.h>
#include <sstream>
#include <KnownFolders.h>
#include <ShlObj.h>
#endif

#include <Editor/EditorManager.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/Logger.hpp>

#include "CSharpProjectBuilder.hpp"
#include "SolutionBuilder.hpp"
#include "CSharpBuildManager.hpp"

using namespace Grindstone::Editor::ScriptBuilder;

static bool IsProgramAvailable(const std::string& programName) {
#if defined(_WIN32)
	std::string command = "where " + programName + " >nul 2>&1";
#else
	std::string command = "command -v " + programName + " >/dev/null 2>&1";
#endif
	int result = std::system(command.c_str());
	return result == 0;
}

static std::string CallProcessAndReadResult(const std::wstring& applicationName, const std::wstring& commandLine) {
	PROCESS_INFORMATION procInfo{};
	HANDLE hStdInPipeRead = nullptr;
	HANDLE hStdInPipeWrite = nullptr;
	HANDLE hStdOutPipeRead = nullptr;
	HANDLE hStdOutPipeWrite = nullptr;

	// Create two pipes.
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), nullptr, true };
	if (!CreatePipe(&hStdInPipeRead, &hStdInPipeWrite, &sa, 0)) {
		return "";
	}

	if (!CreatePipe(&hStdOutPipeRead, &hStdOutPipeWrite, &sa, 0)) {
		return "";
	}

	STARTUPINFOW startInfo{};
	startInfo.cb = sizeof(STARTUPINFOW);
	startInfo.hStdInput = hStdInPipeRead;
	startInfo.hStdError = hStdOutPipeWrite;
	startInfo.hStdOutput = hStdOutPipeWrite;
	startInfo.dwFlags = STARTF_USESTDHANDLES;

	const LPCWSTR appName = !applicationName.empty()
		? applicationName.c_str()
		: nullptr;

	const LPWSTR cmdline = !commandLine.empty()
		? const_cast<LPWSTR>(commandLine.c_str())
		: nullptr;

	constexpr PROCESS_INFORMATION processInfo{};

	if (CreateProcessW(appName, cmdline, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startInfo, &procInfo)) {
		WaitForSingleObject(processInfo.hProcess, INFINITE);
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
		CloseHandle(hStdOutPipeWrite);
		CloseHandle(hStdInPipeRead);

		std::string outputText;

		constexpr int bufferSize = 512;

		CHAR buf[bufferSize + 1];
		DWORD n;
		while (ReadFile(hStdOutPipeRead, buf, bufferSize, &n, nullptr)) {
			buf[n] = 0;
			outputText.append(buf);
		}

		CloseHandle(hStdOutPipeRead);
		CloseHandle(hStdInPipeWrite);

		return outputText;
	}

	return "";
}

void CSharpBuildManager::FinishInitialFileProcessing() const {
	CreateProjectsAndSolution();
}

void CSharpBuildManager::AddFileInitial(const std::filesystem::path& path) {
	files.emplace_back(path);
}

void CSharpBuildManager::OnFileAdded(const std::filesystem::path& path) {
	files.emplace_back(path);
	CreateProjectsAndSolution();
}

void CSharpBuildManager::OnFileMoved(
	const std::filesystem::path& updatedPath,
	const std::filesystem::path& originalPath
) {
	for (std::filesystem::path& filepath : files) {
		if (filepath == originalPath) {
			filepath = updatedPath;
			CreateProjectsAndSolution();
		}
	}

	CreateProjectsAndSolution();
}

void CSharpBuildManager::OnFileDeleted(const std::filesystem::path& path) {
	for (size_t i = 0; i < files.size(); ++i) {
		std::filesystem::path& file = files[i];
		if (file == path) {
			const auto iterator = files.begin() + i;
			files.erase(iterator);
			CreateProjectsAndSolution();
		}
	}
}

void CSharpBuildManager::OnFileModified(const std::filesystem::path& path) {
	BuildProject();
}

#ifdef _MSC_VER
PROCESS_INFORMATION dotnetProcessInfo;
HANDLE hStdOutPipeRead = nullptr;
HANDLE hStdOutPipeWrite = nullptr;

// TODO: Multi-thread this with DWORD __stdcall ReadDataFromExtProgram(void* argh) {
static DWORD ReadDataFromExtProgram(const std::string& path) {
	CloseHandle(hStdOutPipeWrite);
	GPRINT_INFO_V(Grindstone::LogSource::Editor, "Building user project \"{}\"...", path.c_str());

	for (;;) {
		DWORD bytesAvail = 0;
		if (!PeekNamedPipe(hStdOutPipeRead, nullptr, 0, nullptr, &bytesAvail, nullptr)) {
			break;
		}
		if (bytesAvail) {
			constexpr int bufferSize = 4096;
			CHAR buf[bufferSize + 1];
			DWORD n;
			const BOOL success = ReadFile(hStdOutPipeRead, buf, bufferSize, &n, nullptr);
			if (!success || n == 0) {
				GPRINT_ERROR(Grindstone::LogSource::Editor, "Failed to call ReadFile");
			}

			static const std::regex errorRegex(R"(\): error CS)");
			static const std::regex warningRegex(R"(\): warning CS)");

			std::string errorMsg(buf, buf + n);
			errorMsg = Grindstone::Utils::Trim(errorMsg);
			Grindstone::LogSeverity severity = Grindstone::LogSeverity::Info;
			if (std::regex_search(errorMsg, errorRegex)) {
				severity = Grindstone::LogSeverity::Error;
			}
			else if (std::regex_search(errorMsg, warningRegex)) {
				severity = Grindstone::LogSeverity::Warning;
			}

			GPRINT(severity, Grindstone::LogSource::Editor, errorMsg.c_str());
		}
	}

	GPRINT_INFO_V(Grindstone::LogSource::Editor, "Done building user project \"{}\".", path.c_str());
	CloseHandle(dotnetProcessInfo.hProcess);
	CloseHandle(dotnetProcessInfo.hThread);

	Grindstone::Editor::Manager::GetEngineCore().ReloadCsharpBinaries();

	return 0;
}

static bool CreateChildProcess() {
#if _DEBUG
	constexpr const char* configuration = "Debug";
#else
	constexpr const char* configuration = "Release";
#endif

	const std::string filename = "Application-CSharp.csproj";
	const std::filesystem::path outputFilePath = Grindstone::Editor::Manager::GetInstance().GetProjectPath() / filename;
	const std::string path = outputFilePath.string();

	std::string command = fmt::format("dotnet build {} -c {}", path.c_str(), configuration);

	dotnetProcessInfo = {};
	STARTUPINFO startInfo{};
	startInfo.cb = sizeof(STARTUPINFO);
	startInfo.hStdError = hStdOutPipeWrite;
	startInfo.hStdOutput = hStdOutPipeWrite;
	startInfo.dwFlags |= STARTF_USESTDHANDLES;

	if (CreateProcessA(
		nullptr,
		const_cast<LPSTR>(command.c_str()),
		nullptr,
		nullptr,
		TRUE,
		0,
		nullptr,
		nullptr,
		&startInfo,
		&dotnetProcessInfo
	)) {
		// TODO: Multi-thread this with CreateThread(0, 0, ReadDataFromExtProgram, nullptr, 0, nullptr);
		ReadDataFromExtProgram(path);
		return true;
	}

	GPRINT_ERROR_V(Grindstone::LogSource::Editor, "Unable to build project: {}", command.c_str());
	return false;
}
#endif

void CSharpBuildManager::BuildProject() {
#ifdef _MSC_VER
	SECURITY_ATTRIBUTES saAttr{};
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = nullptr;

	if (!CreatePipe(&hStdOutPipeRead, &hStdOutPipeWrite, &saAttr, 0)) {
		return;
	}

	if (!SetHandleInformation(hStdOutPipeRead, HANDLE_FLAG_INHERIT, 0)) {
		return;
	}

	CreateChildProcess();
#endif
}

void CSharpBuildManager::CreateProjectsAndSolution() const {
	if (!IsProgramAvailable("dotnet")) {
		GPRINT_ERROR(LogSource::Editor, "Could not get dotnet path - will not be able to compile C# projects.");
		return;
	}

	const CSharpProjectMetaData metaData("Application-CSharp", "64EE0D0C-BB03-4061-B3DE-AADE8705E344");
	CreateProject(metaData);
	CreateSolution(metaData);

	BuildProject();
}

void CSharpBuildManager::CreateProject(const CSharpProjectMetaData& metaData) const {
	CSharpProjectBuilder projectBuilder(metaData);

	for (auto& path : files) {
		projectBuilder.AddCodeFile(path);
	}

	projectBuilder.CreateProject();
}

void CSharpBuildManager::CreateSolution(const CSharpProjectMetaData& metaData) {
	SolutionBuilder solutionBuilder;
	solutionBuilder.AddProject(metaData);
	solutionBuilder.CreateSolution();
}
