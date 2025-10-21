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
#include <Editor/PluginSystem/EditorPluginInterface.hpp>
#include <Editor/PluginSystem/EditorPluginManager.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Scenes/Manager.hpp>
#include <EngineCore/Events/Dispatcher.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/WorldContext/WorldContextManager.hpp>
#include <EngineCore/Logger.hpp>

#include "EditorCamera.hpp"
#include "AssetRegistry.hpp"
#include "AssetTemplateRegistry.hpp"
#include "EditorManager.hpp"
#include "FileAssetLoader.hpp"
#include "FileManager.hpp"
#include "GitManager.hpp"
#include "Selection.hpp"
#include "TaskSystem.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor;
using namespace Grindstone::Memory;

static Grindstone::Editor::Manager* editorManagerInstance = nullptr;

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

Grindstone::Editor::ThumbnailManager& Manager::GetThumbnailManager() {
	return thumbnailManager;
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

	fileManager.Initialize();

	if (!LoadEngine()) {
		return false;
	}

	if (!SetupImguiEditor()) {
		return false;
	}

	engineCore->GetPluginManager()->LoadPluginsByStage("EditorEarly");

	std::string materialContent = "{\n\t\"name\": \"New Material\",\n\t\"shader\": \"\"\n}";
	assetTemplateRegistry.RegisterTemplate(
		AssetType::Material,
		"Material", ".gmat",
		reinterpret_cast<const void*>(materialContent.c_str()), materialContent.size()
	);

	assetRegistry.Initialize(projectPath);
	importerManager.Initialize();

	engineCore->GetPluginManager()->LoadPluginsByStage("EditorAssetImportEarly");
	thumbnailManager.Initialize();
	engineCore->GetPluginManager()->LoadPluginsByStage("EditorAssetImportLate");

	while (taskSystem.HasRunningTasks()) {
		Sleep(100);
	}
	taskSystem.CullDoneTasks();

	// TODO: This might not be necessary or desirable - make it an option.
	fileManager.CleanupStaleFiles();

	assetRegistry.WriteFile();
	gitManager.Initialize();

	engineCore->GetPluginManager()->LoadPluginsByStage("EditorBeforeCameraInitialization");
	Grindstone::Editor::EditorCamera::SetupRenderPasses();
	engineCore->GetPluginManager()->LoadPluginsByStage("EditorAfterCameraInitialization");

	editorWorldContext = engineCore->GetWorldContextManager()->Create();

	engineCore->GetPluginManager()->LoadPluginsByStage("EditorBeforeSceneInitialization");
	engineCore->InitializeScene(true);
	engineCore->GetPluginManager()->LoadPluginsByStage("EditorAfterSceneInitialization");

	engineCore->ShowMainWindow();

	InitializeQuitCommands();

	imguiEditor->CreateWindows();

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

		auto editorPluginManager = static_cast<Grindstone::Plugins::EditorPluginManager*>(engineCore->GetPluginManager());
		editorPluginManager->ProcessQueuedPluginInstallsAndUninstalls();
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
		worldContextManager->Remove(runtimeWorldContext);
		runtimeWorldContext = nullptr;
		worldContextManager->SetActiveWorldContextSet(editorWorldContext);
	}
	else if (newPlayMode == PlayMode::Play && playMode == PlayMode::Editor) {
		runtimeWorldContext = worldContextManager->Create();
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

using CreateEngineFunction = Grindstone::EngineCore*();
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

	Grindstone::EngineCore::EarlyCreateInfo earlyCreateInfo;
	earlyCreateInfo.isEditor = true;
	earlyCreateInfo.applicationModuleName = "GrindstoneGameEditor";
	earlyCreateInfo.applicationTitle = "Grindstone Game Editor";
	const std::string projectPathAsStr = projectPath.string();
	earlyCreateInfo.projectPath = projectPathAsStr.c_str();
	const std::string currentPath = std::filesystem::current_path().string();
	earlyCreateInfo.engineBinaryPath = currentPath.c_str();

	engineCore = createEngineFn();

	if (engineCore == nullptr) {
		return false;
	}

	Grindstone::EngineCore::SetInstance(*engineCore);

	if (!engineCore->EarlyInitialize(earlyCreateInfo)) {
		return false;
	}

	Plugins::Interface* pluginInterface = engineCore->GetPluginInterface();
	Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());
	pluginInterface->SetEditorInterface(Grindstone::Memory::AllocatorCore::Allocate<Grindstone::Plugins::EditorPluginInterface>());
	Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
	Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());

	EngineCore::LateCreateInfo lateCreateInfo;
	lateCreateInfo.assetLoader = Grindstone::Memory::AllocatorCore::Allocate<Assets::FileAssetLoader>();
	Grindstone::Plugins::EditorPluginManager* pluginManager = Grindstone::Memory::AllocatorCore::Allocate<Grindstone::Plugins::EditorPluginManager>();
	lateCreateInfo.pluginManagerOverride = pluginManager;
	pluginManager->AddPluginsFolder(Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins");
	pluginManager->AddPluginsFolder(Grindstone::Editor::Manager::GetInstance().GetProjectPath() / "plugins");
	
	if (!engineCore->Initialize(lateCreateInfo)) {
		return false;
	}

	return true;
}

Manager::~Manager() {
	if (engineCore != nullptr && engineCore->GetGraphicsCore() != nullptr) {
		engineCore->GetGraphicsCore()->WaitUntilIdle();
	}

	if (imguiEditor) {
		AllocatorCore::Free(imguiEditor);
		imguiEditor = nullptr;
	}

	if (engineCore != nullptr) {
		engineCore->GetPluginManager()->UnloadPluginsByStage("EditorAfterSceneInitialization");
		engineCore->GetPluginManager()->UnloadPluginsByStage("EditorBeforeSceneInitialization");
		engineCore->GetPluginManager()->UnloadPluginsByStage("EditorAfterCameraInitialization");
		engineCore->GetPluginManager()->UnloadPluginsByStage("EditorBeforeCameraInitialization");
		engineCore->GetPluginManager()->UnloadPluginsByStage("EditorAssetImportLate");
		engineCore->GetPluginManager()->UnloadPluginsByStage("EditorAssetImportEarly");
		engineCore->GetPluginManager()->UnloadPluginsByStage("EditorEarly");

		engineCore->GetSceneManager()->CloseActiveScenes();

		Grindstone::WorldContextManager* cxtManager = engineCore->GetWorldContextManager();
		if (cxtManager) {
			if (runtimeWorldContext != nullptr) {
				cxtManager->Remove(runtimeWorldContext);
			}

			if (editorWorldContext != nullptr) {
				cxtManager->Remove(editorWorldContext);
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
}
