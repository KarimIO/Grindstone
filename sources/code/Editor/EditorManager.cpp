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

CommandList& Manager::getCommandList() {
	return commandList;
}

bool Manager::initialize() {
	if (!loadEngine())			return false;
	if (!setupImguiEditor())	return false;

	return true;
}

bool Manager::setupImguiEditor() {
	imguiEditor = new ImguiEditor::ImguiEditor(engineCore);
	return true;
}

void Manager::run() {
	while (true) {
		engineCore->runLoopIteration();
		imguiEditor->update();
		engineCore->updateWindows();
	}
}

using CreateEngineFunction = EngineCore*(EngineCore::CreateInfo&);
bool Manager::loadEngine() {
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