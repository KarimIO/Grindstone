#include <Common/Utilities/ModuleLoading.hpp>
#include "ModelConverter.hpp"
#include <EngineCore/EngineCore.hpp>
#include "EditorManager.hpp"
#include "IEditor.hpp"

using namespace Grindstone;

bool Editor::Manager::initialize() {
	if (!loadEngine())				return false;
	if (!renderer_.initialize())	return false;
	if (!addDefaultPlugins())       return false;
	if (!createDefaultEditors())    return false;

	is_running_ = true;

	return true;
}

void Editor::Manager::run() {
	while (is_running_) {
		// Editor Updates
		for (auto* editor : editors_) {
			editor->update();
		}

		// Editor Rendering
		renderer_.render();
	}
}

bool Editor::Manager::loadEngine() {
	Grindstone::Utilities::Modules::Handle handle;
	handle = Grindstone::Utilities::Modules::load("EngineCore");

	if (handle) {
		void* f = Grindstone::Utilities::Modules::getFunction(handle, "runEngine");
		if (f) {
			auto fptr = (void (*)(EngineCore::CreateInfo&))f;

			EngineCore::CreateInfo create_info;
			create_info.is_editor_ = false;
			create_info.application_module_name_ = "ApplicationDLL";
			create_info.application_title_ = "Grindstone Editor";
			fptr(create_info);

			Grindstone::Utilities::Modules::unload(handle);
			return true;
		}
		else {
			std::cerr << "Failed to load runEngine in EngineCore Module.";
		}
	}
	else {
		std::cerr << "Failed to load EngineCore Module.";
	}

	return false;
}

bool Editor::Manager::addDefaultPlugins() {
	// Open default plugins file

	// If it doesn't exist, create one with just AssetBrowser plugin

	// Load plugins from file

	// Close file

	return true;
}

bool Editor::Manager::createDefaultEditors() {
	// Open default loaded plugins file

	// If it doesn't exist, create one with just AssetBrowser plugin

	// Load plugins from file

	// Close file

	return true;
}
