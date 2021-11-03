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

EngineCore& Manager::GetEngineCore() {
	return *GetInstance().engineCore;
}

bool Manager::Initialize() {
	if (!LoadEngine())			return false;
	fileManager.Initialize();
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

void Manager::Print(LogSeverity logSeverity, const char* textFormat, ...) {
	va_list args;
	va_start(args, textFormat);
	GetInstance().engineCore->Print(logSeverity, textFormat, args);
	va_end(args);
}

using CreateEngineFunction = EngineCore*(EngineCore::CreateInfo&);
bool Manager::LoadEngine() {
	Grindstone::Utilities::Modules::Handle handle;
	handle = Grindstone::Utilities::Modules::load("EngineCore");

	if (handle == nullptr) {
		std::cerr << "Failed to load EngineCore Module.\n";
		return false;
	}
	
	CreateEngineFunction* createEngineFn =
		(CreateEngineFunction*)Utilities::Modules::getFunction(handle, "createEngine");
	if (createEngineFn == nullptr) {
		std::cerr << "Failed to load runEngine in EngineCore Module.\n";
		return false;
	}

	EngineCore::CreateInfo createInfo;
	createInfo.isEditor = true;
	createInfo.applicationModuleName = "ApplicationDLL";
	createInfo.applicationTitle = "Grindstone Editor";
	engineCore = createEngineFn(createInfo);

	return engineCore != nullptr;
}

Manager::~Manager() {
	/*if (handle) {
		Grindstone::Utilities::Modules::unload(handle);
		handle = 0;
	}*/
}
