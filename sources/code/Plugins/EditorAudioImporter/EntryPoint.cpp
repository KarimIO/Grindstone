#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "AudioImporter.hpp"

using namespace Grindstone;

extern "C" {
	EDITOR_AUDIO_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		AddImporterFactory("wav", ImportAudio);
	}

	EDITOR_AUDIO_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
