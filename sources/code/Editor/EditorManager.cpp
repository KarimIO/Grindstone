#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <Windows.h>

#include <Common/Event/BaseEvent.hpp>
#include <Common/Event/EventType.hpp>
#include <Common/Event/KeyEvent.hpp>
#include <Common/Event/KeyPressCode.hpp>
#include <Common/Input/CursorMode.hpp>
#include <Common/Input/InputInterface.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/Utilities/ModuleLoading.hpp>
#include <Editor/Commands/CommandList.hpp>
#include <Editor/ImguiEditor/ImguiEditor.hpp>
#include <Editor/Importers/ImporterManager.hpp>
#include <Editor/ScriptBuilder/CSharpBuildManager.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Events/Dispatcher.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/PluginSystem/Manager.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/WorldContext/WorldContextManager.hpp>
#include <EngineCore/Logger.hpp>

#include "AssetRegistry.hpp"
#include "AssetTemplateRegistry.hpp"
#include "EditorManager.hpp"
#include "EditorPluginInterface.hpp"
#include "FileAssetLoader.hpp"
#include "FileManager.hpp"
#include "GitManager.hpp"
#include "Selection.hpp"
#include "TaskSystem.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor;
using namespace Grindstone::Memory;

Manager& Manager::GetInstance() {
	static Editor::Manager manager;
	return manager;
}

Grindstone::Importers::ImporterManager& Manager::GetImporterManager() {
	return importerManager;
}

ImguiEditor::ImguiEditor& Manager::GetImguiEditor() {
	return *imguiEditor;
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

AssetTemplateRegistry& Manager::GetAssetTemplateRegistry() {
	return assetTemplateRegistry;
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

	std::string materialContent = "{\n\t\"name\": \"New Material\",\n\t\"shader\": \"\"\n}";
	assetTemplateRegistry.RegisterTemplate(
		AssetType::Material,
		"Material", ".gmat",
		reinterpret_cast<const void*>(materialContent.c_str()), materialContent.size()
	);

	assetRegistry.Initialize(projectPath);

	fileManager.WatchDirectory("ASSETS", assetsPath);
	fileManager.WatchDirectory("ENGINE", engineBinariesPath.parent_path() / "engineassets");

	while (taskSystem.HasRunningTasks()) {
		Sleep(100);
	}
	taskSystem.CullDoneTasks();

	// TODO: This might not be necessary or desirable - make it an option.
	fileManager.CleanupStaleFiles();

	assetRegistry.WriteFile();

	gitManager.Initialize();
	csharpBuildManager.FinishInitialFileProcessing();

	if (!SetupImguiEditor()) {
		return false;
	}

	editorWorldContext = &engineCore->GetWorldContextManager()->Create();
	engineCore->InitializeScene(true);
	engineCore->ShowMainWindow();

	InitializeQuitCommands();

	return true;
}

void Manager::InitializeQuitCommands() {
	Events::Dispatcher* dispatcher = engineCore->GetEventDispatcher();
	dispatcher->AddEventListener(Grindstone::Events::EventType::KeyPress, std::bind(&Manager::OnKeyPress, this, std::placeholders::_1));
	dispatcher->AddEventListener(Grindstone::Events::EventType::WindowTryQuit, std::bind(&Manager::OnTryQuit, this, std::placeholders::_1));
	dispatcher->AddEventListener(Grindstone::Events::EventType::WindowForceQuit, std::bind(&Manager::OnForceQuit, this, std::placeholders::_1));
}

bool Manager::SetupImguiEditor() {
	imguiEditor = AllocatorCore::Allocate<ImguiEditor::ImguiEditor>(engineCore);
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
		case PlayMode::Pause:
			break;
		}

		imguiEditor->Update();
		engineCore->UpdateWindows();

		if (newPlayMode != playMode) {
			TransferPlayMode(newPlayMode);
		}
	}
}

void Manager::SetPlayMode(PlayMode newPlayMode) {
	this->newPlayMode = newPlayMode;
}

void Manager::TransferPlayMode(PlayMode newPlayMode) {
	// Reset input before setting the new playMode
	engineCore->GetInputManager()->SetCursorIsRawMotion(false);
	engineCore->GetInputManager()->SetCursorMode(Input::CursorMode::Normal);

	Grindstone::WorldContextManager* worldContextManager = engineCore->GetWorldContextManager();
	Grindstone::ECS::ComponentRegistrar* componentRegistry = engineCore->GetComponentRegistrar();
	if (newPlayMode == PlayMode::Editor && playMode == PlayMode::Play) {
		componentRegistry->CallDestroyOnRegistry(*runtimeWorldContext);
		worldContextManager->Remove(*runtimeWorldContext);
		worldContextManager->SetActiveWorldContextSet(editorWorldContext);
	}
	else if (newPlayMode == PlayMode::Play && playMode == PlayMode::Editor) {
		runtimeWorldContext = &worldContextManager->Create();
		componentRegistry->CopyRegistry(*runtimeWorldContext, *editorWorldContext);
		componentRegistry->CallCreateOnRegistry(*editorWorldContext);
		worldContextManager->SetActiveWorldContextSet(runtimeWorldContext);
	}

	selection.Clear();

	playMode = newPlayMode;
}

PlayMode Manager::GetPlayMode() const {
	return playMode;
}

const std::filesystem::path& Manager::GetProjectPath() const {
	return projectPath;
}

const std::filesystem::path& Manager::GetAssetsPath() const {
	return assetsPath;
}

const std::filesystem::path& Manager::GetCompiledAssetsPath() const {
	return compiledAssetsPath;
}

const std::filesystem::path& Grindstone::Editor::Manager::GetEngineBinariesPath() const {
	return engineBinariesPath;
}

bool Manager::OnKeyPress(Grindstone::Events::BaseEvent* ev) {
	const Events::KeyPressEvent* onKeyPressEvent = dynamic_cast<Events::KeyPressEvent*>(ev);

	if (onKeyPressEvent == nullptr) {
		return false;
	}

	switch (onKeyPressEvent->code) {
		case Events::KeyPressCode::Escape:
			if (GetPlayMode() != Editor::PlayMode::Editor) {
				SetPlayMode(Editor::PlayMode::Editor);
				return true;
			}
			break;
		default: ;
	}

	return false;
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
		static_cast<CreateEngineFunction*>(Utilities::Modules::GetFunction(engineCoreLibraryHandle, "CreateEngine"));
	if (createEngineFn == nullptr) {
		std::cerr << "Failed to load CreateEngine in EngineCore Module.\n";
		return false;
	}

	EngineCore::CreateInfo createInfo;
	createInfo.isEditor = true;
	createInfo.applicationModuleName = "GrindstoneGameEditor";
	createInfo.applicationTitle = "Grindstone Game Editor";
	const std::string projectPathAsStr = projectPath.string();
	createInfo.projectPath = projectPathAsStr.c_str();
	createInfo.assetLoader = new Assets::FileAssetLoader();
	createInfo.editorPluginInterface = new Grindstone::Plugins::EditorPluginInterface();

	const std::string currentPath = std::filesystem::current_path().string();
	createInfo.engineBinaryPath = currentPath.c_str();
	engineCore = createEngineFn(createInfo);

	Plugins::Interface& pluginInterface = engineCore->GetPluginManager()->GetInterface();
	Grindstone::Logger::SetLoggerState(pluginInterface.GetLoggerState());
	Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface.GetAllocatorState());

	Grindstone::EngineCore::SetInstance(*engineCore);

	return engineCore != nullptr;
}

Manager::~Manager() {
	engineCore->GetGraphicsCore()->WaitUntilIdle();

	if (imguiEditor) {
		AllocatorCore::Free(imguiEditor);
		imguiEditor = nullptr;
	}

	Grindstone::WorldContextManager* cxtManager = engineCore->GetWorldContextManager();
	if (cxtManager) {
		if (runtimeWorldContext) {
			cxtManager->Remove(*runtimeWorldContext);
		}

		if (editorWorldContext) {
			cxtManager->Remove(*editorWorldContext);
		}
	}

	if (engineCoreLibraryHandle) {
		using DestroyEngineFunction = void *();

		DestroyEngineFunction* destroyEngineFn =
			static_cast<DestroyEngineFunction*>(Utilities::Modules::GetFunction(engineCoreLibraryHandle, "DestroyEngine"));
		if (destroyEngineFn != nullptr) {
			destroyEngineFn();
		}

		Grindstone::Utilities::Modules::Unload(engineCoreLibraryHandle);
		engineCoreLibraryHandle = nullptr;
	}
}
