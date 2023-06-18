#include "Editor/EditorManager.hpp"
#include "CSharpBuildManager.hpp"
#include "CSharpProjectBuilder.hpp"
#include "SolutionBuilder.hpp"
#include "EngineCore/Utils/Utilities.hpp"

#include <thread>
#include <algorithm>

#ifdef _MSC_VER
#include <Windows.h>
#endif

using namespace Grindstone::Editor::ScriptBuilder;

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
	const std::filesystem::path& originalPath,
	const std::filesystem::path& updatedPath
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
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

std::string GetMsBuildPath() {
	std::filesystem::path settingsFile = Grindstone::Editor::Manager::GetInstance().GetProjectPath() / "userSettings/codeToolsPath.txt";
	std::filesystem::create_directories(settingsFile.parent_path());
	auto settingsPath = settingsFile.string();
	return Grindstone::Utils::LoadFileText(settingsPath.c_str());
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
		if (!PeekNamedPipe(g_hChildStd_OUT_Rd, NULL, 0, NULL, &bytesAvail, NULL)) {
			break;
		}
		if (bytesAvail) {
			CHAR buf[BUFSIZE];
			DWORD n;
			BOOL success = ReadFile(g_hChildStd_OUT_Rd, buf, BUFSIZE, &n, NULL);
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
	LPSTR pathLpcstr = const_cast<char*>(path.c_str());
	std::string msBuildPath = GetMsBuildPath();

	std::string command = std::string("\"") + msBuildPath + "\" " + parameters + " \"" + path + "\"";

	processInfo = {};
	STARTUPINFO startInfo{};
	BOOL bSuccess = FALSE;
	startInfo.cb = sizeof(STARTUPINFO);
	startInfo.hStdError = g_hChildStd_OUT_Wr;
	startInfo.hStdOutput = g_hChildStd_OUT_Wr;
	startInfo.dwFlags |= STARTF_USESTDHANDLES;

	if (CreateProcess(NULL, (TCHAR*)command.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &startInfo, &processInfo)) {
		// Multithreaded: CreateThread(0, 0, ReadDataFromExtProgram, NULL, 0, NULL);
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
	saAttr.lpSecurityDescriptor = NULL;

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
	if (msBuildPath.empty() || !std::filesystem::exists(msBuildPath.c_str())) {
		Grindstone::Editor::Manager::GetInstance().Print(Grindstone::LogSeverity::Error, "Invalid MsBuild Path");
		return;
	}

	CSharpProjectMetaData metaData("Application-CSharp", "64EE0D0C-BB03-4061-B3DE-AADE8705E344");
	CreateProject(metaData);
	CreateSolution(metaData);

	BuildProject();
}

void CSharpBuildManager::CreateProject(CSharpProjectMetaData metaData) {
	CSharpProjectBuilder projectBuilder(metaData);

	for (auto& path : files) {
		projectBuilder.AddCodeFile(path);
	}
	
	projectBuilder.CreateProject();
}

void CSharpBuildManager::CreateSolution(CSharpProjectMetaData metaData) {
	SolutionBuilder solutionBuilder;
	solutionBuilder.AddProject(metaData);
	solutionBuilder.CreateSolution();
}
