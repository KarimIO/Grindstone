#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "ShaderImporter.hpp"

using namespace Grindstone;

extern "C" {
	EDITOR_SHADER_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		AddImporterFactory("glsl", ImportShadersFromGlsl);
	}

	EDITOR_SHADER_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
