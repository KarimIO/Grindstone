#include "CSharpBuildManager.hpp"

#include <fstream>

#ifdef _MSC_VER
#include <Windows.h>
#include <sstream>
#include <KnownFolders.h>
#include <ShlObj.h>
#endif

#include "Editor/EditorManager.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include <EngineCore/Logger.hpp>
#include "CSharpProjectBuilder.hpp"
#include "SolutionBuilder.hpp"

using namespace Grindstone::Editor::ScriptBuilder;

std::string CallProcessAndReadResult(const std::wstring& applicationName, const std::wstring& commandLine) {
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
PROCESS_INFORMATION msBuildProcessInfo;
HANDLE hStdOutPipeRead = nullptr;
HANDLE hStdOutPipeWrite = nullptr;

static std::string GetMsBuildPath() {
	const std::filesystem::path settingsFile = Grindstone::Editor::Manager::GetInstance().GetProjectPath() / "userSettings/codeToolsPath.txt";
	std::filesystem::create_directories(settingsFile.parent_path());
	const auto settingsPath = settingsFile.string();
	std::string msbuildPath = Grindstone::Utils::LoadFileText(settingsPath.c_str());
	msbuildPath = Grindstone::Utils::Trim(msbuildPath);

	if (!msbuildPath.empty()) {
		return msbuildPath;
	}

	wchar_t* programFilesPath = nullptr;
	if (SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, 0, nullptr, &programFilesPath) != S_OK) {
		return "";
	}

	std::wstringstream vsWherePathSStream;
	vsWherePathSStream << '\"' << programFilesPath << L"\\Microsoft Visual Studio\\Installer\\vswhere.exe\" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\\**\\Bin\\MSBuild.exe";
	const std::wstring commandLine = vsWherePathSStream.str();

	CoTaskMemFree(static_cast<void*>(programFilesPath));

	std::string searchedPath = CallProcessAndReadResult(L"", commandLine);

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
		// Write the path before returning it.
		std::ofstream outputFile(settingsPath.c_str());

		if (outputFile.is_open()) {
			outputFile.clear();
			outputFile << searchedPath << '\n';
			outputFile.close();
		}

		return searchedPath;
	}

	return "";
}

// TODO: Multi-thread this with DWORD __stdcall ReadDataFromExtProgram(void* argh) {
static DWORD ReadDataFromExtProgram() {
	CloseHandle(hStdOutPipeWrite);
	GPRINT_INFO(Grindstone::LogSource::Editor, "Building...");

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

			std::string errorMsg(buf, buf + n);
			GPRINT_INFO(Grindstone::LogSource::Editor, errorMsg.c_str());
		}
	}

	GPRINT_INFO(Grindstone::LogSource::Editor, "Done building.");
	CloseHandle(msBuildProcessInfo.hProcess);
	CloseHandle(msBuildProcessInfo.hThread);

	Grindstone::Editor::Manager::GetEngineCore().ReloadCsharpBinaries();

	return 0;
}

bool CreateChildProcess() {
	constexpr bool isDebug = true;
	std::string parameters = "-noLogo -noAutoRsp -verbosity:minimal /p:Configuration=";
	if (isDebug) {
		parameters += "Debug";
	}
	else {
		parameters += "Release";
	}

	const std::string filename = "Application-CSharp.csproj";
	const std::filesystem::path outputFilePath = Grindstone::Editor::Manager::GetInstance().GetProjectPath() / filename;
	const std::string path = outputFilePath.string();
	const std::string msBuildPath = GetMsBuildPath();

	std::stringstream commandSStream;
	commandSStream << '\"' << msBuildPath << "\" " << parameters << " \"" << path << '\"';
	std::string command = commandSStream.str();

	msBuildProcessInfo = {};
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
		&msBuildProcessInfo
	))
	{
		// TODO: Multi-thread this with CreateThread(0, 0, ReadDataFromExtProgram, nullptr, 0, nullptr);
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
	const std::string msBuildPath = GetMsBuildPath();
	if (msBuildPath.empty()) {
		GPRINT_ERROR(LogSource::Editor, "Could not get visual studio path.");
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
