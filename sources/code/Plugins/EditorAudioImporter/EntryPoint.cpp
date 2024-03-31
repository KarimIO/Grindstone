#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "AudioImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::Importers;

extern "C" {
	EDITOR_AUDIO_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		pluginInterface->EditorRegisterAssetImporter("wav", ImportAudio);
	}

	EDITOR_AUDIO_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->EditorDeregisterAssetImporter("wav");
	}
}
