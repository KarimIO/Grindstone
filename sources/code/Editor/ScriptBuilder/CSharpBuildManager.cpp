#include "Editor/EditorManager.hpp"
#include "CSharpBuildManager.hpp"
#include "CSharpProjectBuilder.hpp"
#include "SolutionBuilder.hpp"
#include "EngineCore/Utils/Utilities.hpp"

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

HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

void ReadFromPipe(void) {
	constexpr int BUFSIZE = 4096;

	DWORD dwRead;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	std::string content = "";
	for (;;)
	{
		std::memset(chBuf, 0, sizeof(chBuf));
		bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;

		content += chBuf;
	}

	Grindstone::Editor::Manager::Print(Grindstone::LogSeverity::Info, "Error building C# project:\n{}", content.c_str());
}

std::string GetMsBuildPath() {
	std::filesystem::path settingsFile = Grindstone::Editor::Manager::GetInstance().GetProjectPath() / "userSettings/codeToolsPath.txt";
	std::filesystem::create_directories(settingsFile.parent_path());
	auto settingsPath = settingsFile.string();
	return Grindstone::Utils::LoadFileText(settingsPath.c_str());
}

void CreateChildProcess() {
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
	std::string filename = "Application-CSharp.csproj";
	auto outputFilePath = Grindstone::Editor::Manager::GetInstance().GetProjectPath() / filename;
	std::string path = outputFilePath.string();
	std::string msBuildPath = GetMsBuildPath();

	std::string command = msBuildPath + " " + path;

	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 

	bSuccess = CreateProcess(
		NULL,
		(LPSTR)command.c_str(),     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		CREATE_NO_WINDOW,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo
	);  // receives PROCESS_INFORMATION 

	 // If an error occurs, exit the application. 
	if (!bSuccess)
		return;
	else
	{
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);

		CloseHandle(g_hChildStd_OUT_Wr);
	}
}

void CSharpBuildManager::BuildProject() {
	UnloadCsharpBinaries();
#ifdef _MSC_VER
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		return;

	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		return;

	CreateChildProcess();
	ReadFromPipe();
#endif
	ReloadCsharpBinaries();
}

void CSharpBuildManager::UnloadCsharpBinaries() {
	// Editor::Manager::GetEngineCore().UnloadCsharpBinaries();
}

void CSharpBuildManager::ReloadCsharpBinaries() {
	// Editor::Manager::GetEngineCore().ReloadCsharpBinaries();
}

void CSharpBuildManager::CreateProjectsAndSolution() {
	std::string msBuildPath = GetMsBuildPath();
	if (msBuildPath.empty() || !std::filesystem::exists(msBuildPath.c_str())) {
		Grindstone::Editor::Manager::GetInstance().Print(Grindstone::LogSeverity::Error, "Invalid MsBuild Path");
		return;
	}

	CreateProject();
	CreateSolution();

	BuildProject();
}

void CSharpBuildManager::CreateProject() {
	CSharpProjectMetaData metaData("Application-CSharp", "64EE0D0C-BB03-4061-B3DE-AADE8705E344");
	
	CSharpProjectBuilder projectBuilder(metaData);

	for (auto& path : files) {
		projectBuilder.AddCodeFile(path);
	}
	
	projectBuilder.CreateProject();
}

void CSharpBuildManager::CreateSolution() {
	CSharpProjectMetaData metaData("Application-CSharp", "64EE0D0C-BB03-4061-B3DE-AADE8705E344");

	SolutionBuilder solutionBuilder;
	solutionBuilder.AddProject(metaData);
	solutionBuilder.CreateSolution();
}
