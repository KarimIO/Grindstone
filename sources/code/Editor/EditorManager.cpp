#include <iostream>
#include <Common/Utilities/ModuleLoading.hpp>
#include "EngineCore/EngineCore.hpp"
#include "EditorManager.hpp"
#include "ImguiEditor/ImguiEditor.hpp"
using namespace Grindstone;
using namespace Grindstone::Editor;

Manager& Manager::GetInstance() {
	static Editor::Manager manager;
	return manager;
}

CommandList& Manager::GetCommandList() {
	return commandList;
}

Selection& Manager::GetSelection() {
	return selection;
}

EngineCore& Manager::GetEngineCore() {
	return *GetInstance().engineCore;
}

bool Manager::Initialize() {
	if (!LoadEngine())			return false;
	if (!SetupImguiEditor())	return false;

	return true;
}

bool Manager::SetupImguiEditor() {
	imguiEditor = new ImguiEditor::ImguiEditor(engineCore);
	return true;
}

void Manager::Run() {
	while (true) {
		engineCore->RunLoopIteration();
		imguiEditor->Update();
		engineCore->UpdateWindows();
	}
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

	return true;
}

Manager::~Manager() {
	/*if (handle) {
		Grindstone::Utilities::Modules::unload(handle);
		handle = 0;
	}*/
}