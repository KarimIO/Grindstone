#include <iostream>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Events/Dispatcher.hpp"
#include "EditorManager.hpp"
#include "ImguiEditor/ImguiEditor.hpp"
#include "Common/Event/WindowEvent.hpp"
using namespace Grindstone;
using namespace Grindstone::Editor;

Manager& Manager::GetInstance() {
	static Editor::Manager manager;
	return manager;
}

Importers::ImporterManager& Manager::GetImporterManager() {
	return importerManager;
}

AssetRegistry& Grindstone::Editor::Manager::GetAssetRegistry() {
	return assetRegistry;
}

CommandList& Manager::GetCommandList() {
	return commandList;
}

GitManager& Manager::GetGitManager() {
	return gitManager;
}

Selection& Manager::GetSelection() {
	return selection;
}

TaskSystem& Manager::GetTaskSystem() {
	return taskSystem;
}

FileManager& Manager::GetFileManager() {
	return GetInstance().fileManager;
}

ScriptBuilder::CSharpBuildManager& Manager::GetCSharpBuildManager() {
	return csharpBuildManager;
}

EngineCore& Manager::GetEngineCore() {
	return *GetInstance().engineCore;
}

bool Manager::Initialize(std::filesystem::path projectPath) {
	this->projectPath = projectPath;
	assetsPath = this->projectPath / "assets";
	compiledAssetsPath = this->projectPath / "compiledAssets";
	engineBinariesPath = std::filesystem::current_path();

	if (!LoadEngine()) {
		return false;
	}
	assetRegistry.Initialize(projectPath);
	fileManager.Initialize(assetsPath);
	gitManager.Initialize();
	csharpBuildManager.FinishInitialFileProcessing();

	if (!SetupImguiEditor()) {
		return false;
	}

	engineCore->InitializeScene(true);
	engineCore->ShowMainWindow();

	InitializeQuitCommands();

	return true;
}

void Manager::InitializeQuitCommands() {
	auto dispatcher = engineCore->GetEventDispatcher();
	dispatcher->AddEventListener(Grindstone::Events::EventType::WindowTryQuit, std::bind(&Manager::OnTryQuit, this, std::placeholders::_1));
	dispatcher->AddEventListener(Grindstone::Events::EventType::WindowForceQuit, std::bind(&Manager::OnForceQuit, this, std::placeholders::_1));
}

bool Manager::SetupImguiEditor() {
	imguiEditor = new ImguiEditor::ImguiEditor(engineCore);
	return true;
}

void Manager::Run() {
	while (!shouldClose) {
		switch (playMode) {
		case PlayMode::Editor:
			engineCore->RunEditorLoopIteration();
			break;
		case PlayMode::Play:
			engineCore->RunLoopIteration();
			break;
		}

		imguiEditor->Update();
		engineCore->UpdateWindows();
	}
}

void Manager::SetPlayMode(PlayMode newPlayMode) {
	playMode = newPlayMode;
}

PlayMode Manager::GetPlayMode() {
	return playMode;
}

std::filesystem::path Manager::GetProjectPath() {
	return projectPath;
}

std::filesystem::path Manager::GetAssetsPath() {
	return assetsPath;
}

std::filesystem::path Manager::GetCompiledAssetsPath() {
	return compiledAssetsPath;
}

std::filesystem::path Grindstone::Editor::Manager::GetEngineBinariesPath() {
	return engineBinariesPath;
}

bool Manager::OnTryQuit(Grindstone::Events::BaseEvent* ev) {
	shouldClose = true;

	return false;
}

bool Manager::OnForceQuit(Grindstone::Events::BaseEvent* ev) {
	shouldClose = true;

	return true;
}

using CreateEngineFunction = EngineCore*(EngineCore::CreateInfo&);
bool Manager::LoadEngine() {
	engineCoreLibraryHandle = Grindstone::Utilities::Modules::Load("EngineCore");

	if (engineCoreLibraryHandle == nullptr) {
		std::cerr << "Failed to load EngineCore Module.\n";
		return false;
	}
	
	CreateEngineFunction* createEngineFn =
		(CreateEngineFunction*)Utilities::Modules::GetFunction(engineCoreLibraryHandle, "CreateEngine");
	if (createEngineFn == nullptr) {
		std::cerr << "Failed to load CreateEngine in EngineCore Module.\n";
		return false;
	}

	EngineCore::CreateInfo createInfo;
	createInfo.isEditor = true;
	createInfo.applicationModuleName = "GrindstoneGameEditor";
	createInfo.applicationTitle = "Grindstone Game Editor";
	std::string projectPathAsStr = projectPath.string();
	createInfo.projectPath = projectPathAsStr.c_str();

	std::string currentPath = std::filesystem::current_path().string();
	createInfo.engineBinaryPath = currentPath.c_str();
	engineCore = createEngineFn(createInfo);

	return engineCore != nullptr;
}

Manager::~Manager() {
	if (engineCoreLibraryHandle) {
		Grindstone::Utilities::Modules::Unload(engineCoreLibraryHandle);
		engineCoreLibraryHandle = 0;
	}
}
