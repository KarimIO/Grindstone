#include <iostream>
#include <Common/Utilities/ModuleLoading.hpp>
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

CommandList& Manager::GetCommandList() {
	return commandList;
}

Selection& Manager::GetSelection() {
	return selection;
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

bool Manager::Initialize(const char* projectPath) {
	this->projectPath = projectPath;
	assetsPath = this->projectPath / "assets";
	engineBinariesPath = std::filesystem::current_path();

	if (!LoadEngine())			return false;
	fileManager.Initialize(assetsPath);
	// csharpBuildManager.FinishInitialFileProcessing();
	if (!SetupImguiEditor())	return false;
	InitializeQuitCommands();

	return true;
}

void Manager::InitializeQuitCommands() {
	auto dispatcher = engineCore->GetEventDispatcher();
	dispatcher->AddEventListener(Grindstone::Events::EventType::WindowTryQuit, std::bind(&Manager::OnTryQuit, this, std::placeholders::_1));
	dispatcher->AddEventListener(Grindstone::Events::EventType::WindowTryQuit, std::bind(&Manager::OnForceQuit, this, std::placeholders::_1));
}

bool Manager::SetupImguiEditor() {
	imguiEditor = new ImguiEditor::ImguiEditor(engineCore);
	return true;
}

void Manager::Run() {
	while (!shouldClose) {
		engineCore->RunEditorLoopIteration();
		imguiEditor->Update();
		engineCore->UpdateWindows();
	}
}

std::filesystem::path Manager::GetProjectPath() {
	return projectPath;
}

std::filesystem::path Manager::GetAssetsPath() {
	return assetsPath;
}

std::filesystem::path Grindstone::Editor::Manager::GetEngineBinariesPath() {
	return engineBinariesPath;
}

bool Manager::OnTryQuit(Grindstone::Events::BaseEvent* ev) {
	auto castedEv = (Grindstone::Events::WindowTryQuitEvent*)ev;
	shouldClose = true;

	return false;
}

bool Manager::OnForceQuit(Grindstone::Events::BaseEvent* ev) {
	auto castedEv = (Grindstone::Events::WindowTryQuitEvent*)ev;
	shouldClose = true;

	return false;
}

using CreateEngineFunction = EngineCore*(EngineCore::CreateInfo&);
bool Manager::LoadEngine() {
	Grindstone::Utilities::Modules::Handle handle;
	handle = Grindstone::Utilities::Modules::Load("EngineCore");

	if (handle == nullptr) {
		std::cerr << "Failed to load EngineCore Module.\n";
		return false;
	}
	
	CreateEngineFunction* createEngineFn =
		(CreateEngineFunction*)Utilities::Modules::GetFunction(handle, "CreateEngine");
	if (createEngineFn == nullptr) {
		std::cerr << "Failed to load CreateEngine in EngineCore Module.\n";
		return false;
	}

	EngineCore::CreateInfo createInfo;
	createInfo.isEditor = true;
	createInfo.applicationModuleName = "GrindstoneGameEditor";
	createInfo.applicationTitle = "Grindstone Game Editor";
	createInfo.scenePath = "";
	createInfo.shouldLoadSceneFromDefaults = true;
	std::string projectPathAsStr = projectPath.string();
	createInfo.projectPath = projectPathAsStr.c_str();
	engineCore = createEngineFn(createInfo);

	return engineCore != nullptr;
}

Manager::~Manager() {
	/*if (handle) {
		Grindstone::Utilities::Modules::unload(handle);
		handle = 0;
	}*/
}
