#include "Common/ResourcePipeline/MetaFile.hpp"
#include "Importer.hpp"
using namespace Grindstone::Editor::Importers;

Importer::~Importer() {
	if (metaFile) {
		delete metaFile;
	}
}
