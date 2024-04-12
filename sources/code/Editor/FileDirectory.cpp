#include <filesystem>
#include <string>

#include <Common/ResourcePipeline/MetaFile.hpp>

#include "AssetRegistry.hpp"
#include "EditorManager.hpp"
#include "FileDirectory.hpp"

using namespace Grindstone::Editor;

File::File(std::filesystem::directory_entry entry) :
	directoryEntry(entry),
	metaFile(Grindstone::Editor::Manager::GetInstance().GetAssetRegistry(), entry.path()) {}

Directory::Directory(std::filesystem::directory_entry path, Directory* parentDirectory) :
	name(path.path().filename().string()),
	path(path),
	parentDirectory(parentDirectory) {}
