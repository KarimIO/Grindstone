#pragma once

#include <string>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <EditorCommon/Editor/Importer.hpp>

namespace Grindstone::Editor::Importers {
	bool GenerateTextureThumbnail(Grindstone::Uuid uuid);
	bool InitializeTextureThumbnailGenerator();
	void ReleaseTextureThumbnailGenerator();
}
