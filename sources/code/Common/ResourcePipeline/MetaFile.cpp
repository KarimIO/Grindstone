#include "MetaFile.hpp"
using namespace Grindstone;

bool MetaFile::LoadOrCreateFromSourcePath(std::filesystem::path sourcePath) {
	return true;
}

std::filesystem::path MetaFile::GetCompiledFileFromUuid(Uuid uuid) {
	return std::filesystem::path();
}

void MetaFile::UpdateHash() {
}

void MetaFile::AddSubResource() {
}

void MetaFile::UpdateWriteDate() {
}

MetaFile::~MetaFile() {

}
