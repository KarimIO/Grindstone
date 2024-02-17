#include "Editor/EditorManager.hpp"
#include "CSharpBuildManager.hpp"
#include "CSharpProjectBuilder.hpp"
#include "SolutionBuilder.hpp"
#include "EngineCore/Utils/Utilities.hpp"

#include <thread>
#include <algorithm>

#ifdef _MSC_VER
#include <Windows.h>
#include <sstream>
#include <KnownFolders.h>
#include <ShlObj.h>
#endif

using namespace Grindstone::Editor::ScriptBuilder;

std::string CallProcessAndReadResult(std::wstring applicationName, std::wstring commandLine) {
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

	LPCWSTR appName = applicationName.c_str();
	LPWSTR cmdline = const_cast<LPWSTR>(commandLine.c_str());

	const PROCESS_INFORMATION processInfo{};

	if (CreateProcessW(nullptr, cmdline, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startInfo, &procInfo)) {
		WaitForSingleObject(processInfo.hProcess, INFINITE);
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
		CloseHandle(hStdOutPipeWrite);
		CloseHandle(hStdInPipeRead);

		std::string outputText;

		constexpr int bufferSize = 512;

		//Read will return when the buffer is full, or if the pipe on the other end has been broken
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

void CSharpBuildManager::FinishInitialFileProcessing() {
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
	for (int i = 0; i < files.size(); ++i) {
		auto& file = files[i];
		if (file == originalPath) {
			files[i] = updatedPath;
			CreateProjectsAndSolution();
		}
	}

	CreateProjectsAndSolution();
}

void CSharpBuildManager::OnFileDeleted(const std::filesystem::path& path) {
	for (int i = 0; i < files.size(); ++i) {
		auto& file = files[i];
		if (file == path) {
			files.erase(files.begin() + i);
			CreateProjectsAndSolution();
		}
	}
}

void CSharpBuildManager::OnFileModified(const std::filesystem::path& path) {
	BuildProject();
}

#ifdef _MSC_VER
std::thread newThread;
PROCESS_INFORMATION processInfo;
HANDLE g_hChildStd_OUT_Rd = nullptr;
HANDLE g_hChildStd_OUT_Wr = nullptr;

std::string GetMsBuildPath() {
	const std::filesystem::path settingsFile = Grindstone::Editor::Manager::GetInstance().GetProjectPath() / "userSettings/codeToolsPath.txt";
	std::filesystem::create_directories(settingsFile.parent_path());
	const auto settingsPath = settingsFile.string();
	std::string msbuildPath = Grindstone::Utils::LoadFileText(settingsPath.c_str());

	if (!msbuildPath.empty()) {
		return msbuildPath;
	}

	wchar_t* programFilesPath = nullptr;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, 0, nullptr, &programFilesPath);

	if (hr != S_OK) {
		return "";
	}

	std::wstringstream vsWherePathSStream;
	vsWherePathSStream << '\"' << programFilesPath << "\\Microsoft Visual Studio\\Installer\\vswhere.exe\"";
	std::wstring applicationName = vsWherePathSStream.str();
	vsWherePathSStream << L" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\\**\\Bin\\MSBuild.exe";
	std::wstring commandLine = vsWherePathSStream.str();

	CoTaskMemFree(static_cast<void*>(programFilesPath));

	std::string searchedPath = CallProcessAndReadResult(applicationName, commandLine);

	if (searchedPath.size() < 2) {
		return "";
	}

	if (searchedPath.back() == '\n') {
		if (searchedPath[searchedPath.size() - 2] == '\r') {
			searchedPath = searchedPath.substr(0, searchedPath.size() - 2);
		}
		else {
			searchedPath = searchedPath.substr(0, searchedPath.size() - 1);
		}
	}

	if (std::filesystem::exists(searchedPath)) {
		return searchedPath;
	}

	return "";
}

// Multithreaded: DWORD __stdcall ReadDataFromExtProgram(void* argh) {
DWORD ReadDataFromExtProgram() {
	constexpr int BUFSIZE = 4096;

	CHAR chBuf[BUFSIZE];
	memset(chBuf, 0, BUFSIZE);
	BOOL bSuccess = FALSE;

	CloseHandle(g_hChildStd_OUT_Wr);
	Grindstone::Editor::Manager::Print(Grindstone::LogSeverity::Info, "Building...", chBuf);

	for (;;)
	{
		DWORD bytesAvail = 0;
		if (!PeekNamedPipe(g_hChildStd_OUT_Rd, nullptr, 0, nullptr, &bytesAvail, nullptr)) {
			break;
		}
		if (bytesAvail) {
			CHAR buf[BUFSIZE];
			DWORD n;
			BOOL success = ReadFile(g_hChildStd_OUT_Rd, buf, BUFSIZE, &n, nullptr);
			if (!success || n == 0) {
				Grindstone::Editor::Manager::Print(Grindstone::LogSeverity::Error, "Failed to call ReadFile");
			}
			Grindstone::Editor::Manager::Print(Grindstone::LogSeverity::Info, std::string(buf, buf + n));
		}
	}

	Grindstone::Editor::Manager::Print(Grindstone::LogSeverity::Info, "Done building.", chBuf);
	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);

	Grindstone::Editor::Manager::GetEngineCore().ReloadCsharpBinaries();

	return 0;
}

bool CreateChildProcess() {
	bool isDebug = true;
	std::string parameters = "-noLogo -noAutoRsp -verbosity:minimal /p:Configuration=";
	if (isDebug) {
		parameters += "Debug";
	}
	else {
		parameters += "Release";
	}

	std::string filename = "Application-CSharp.csproj";
	auto outputFilePath = Grindstone::Editor::Manager::GetInstance().GetProjectPath() / filename;
	std::string path = outputFilePath.string();
	std::string msBuildPath = GetMsBuildPath();

	std::string command = std::string("\"") + msBuildPath + "\" " + parameters + " \"" + path + "\"";

	processInfo = {};
	STARTUPINFO startInfo{};
	startInfo.cb = sizeof(STARTUPINFO);
	startInfo.hStdError = g_hChildStd_OUT_Wr;
	startInfo.hStdOutput = g_hChildStd_OUT_Wr;
	startInfo.dwFlags |= STARTF_USESTDHANDLES;

	if (CreateProcess(nullptr, (TCHAR*)command.c_str(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startInfo, &processInfo)) {
		// Multithreaded: CreateThread(0, 0, ReadDataFromExtProgram, nullptr, 0, nullptr);
		ReadDataFromExtProgram();
		return true;
	}

	return false;
}
#endif

void CSharpBuildManager::BuildProject() {
#ifdef _MSC_VER
	SECURITY_ATTRIBUTES saAttr{};
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = nullptr;

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) {
		return;
	}

	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
		return;
	}

	CreateChildProcess();
#endif
}

void CSharpBuildManager::CreateProjectsAndSolution() {
	std::string msBuildPath = GetMsBuildPath();
	if (msBuildPath.empty()) {
		Grindstone::Editor::Manager::Print(Grindstone::LogSeverity::Error, "Could not get visual studio path.");
		return;
	}

	const CSharpProjectMetaData metaData("Application-CSharp", "64EE0D0C-BB03-4061-B3DE-AADE8705E344");
	CreateProject(metaData);
	CreateSolution(metaData);

	BuildProject();
}

void CSharpBuildManager::CreateProject(const CSharpProjectMetaData& metaData) {
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
