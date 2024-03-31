#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "ShaderImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::Importers;

extern "C" {
	EDITOR_SHADER_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		pluginInterface->EditorRegisterAssetImporter("glsl", ImportShadersFromGlsl);
	}

	EDITOR_SHADER_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->EditorDeregisterAssetImporter("glsl");
	}
}
