#include "ImporterManager.hpp"
#include "ModelImporter.hpp";
#include "ShaderImporter.hpp";
#include "TextureImporter.hpp";
#include "MaterialImporter.hpp";
using namespace Grindstone::Importers;

ImporterManager::ImporterManager() {
	AddImporterFactory("fbx", ImportModel);
	AddImporterFactory("dae", ImportModel);
	AddImporterFactory("obj", ImportModel);

	AddImporterFactory("png", ImportTexture);
	AddImporterFactory("jpg", ImportTexture);
	AddImporterFactory("tga", ImportTexture);
	AddImporterFactory("jpeg", ImportTexture);
	AddImporterFactory("bmp", ImportTexture);
	AddImporterFactory("psd", ImportTexture);

	AddImporterFactory("glsl", ImportShadersFromGlsl);
}

bool ImporterManager::Import(std::filesystem::path& path) {
	ImporterFactory importerFactory = GetImporterFactoryByPath(path);
	if (importerFactory == nullptr) {
		return false;
	}

	importerFactory(path);
}

void ImporterManager::AddImporterFactory(std::string extension, ImporterFactory importerFactory) {
	if (HasImporter(extension)) {
		return;
	}

	extensionsToImporterFactories[extension] = importerFactory;
}

void ImporterManager::RemoveImporterFactoryByExtension(std::string& extension) {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	if (extensionIterator != extensionsToImporterFactories.end()) {
		extensionsToImporterFactories.erase(extension);
	}
}

ImporterManager::ImporterFactory ImporterManager::GetImporterFactoryByExtension(std::string& extension) {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	return (extensionIterator != extensionsToImporterFactories.end())
		? extensionIterator->second
		: ImporterFactory();
}

ImporterManager::ImporterFactory ImporterManager::GetImporterFactoryByPath(std::filesystem::path& path) {
	std::string extension = path.extension().string();
	return GetImporterFactoryByExtension(extension);
}

bool ImporterManager::HasImporter(std::string& extension) {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	return extensionIterator != extensionsToImporterFactories.end();
}

bool ImporterManager::HasImporter(std::filesystem::path& path) {
	std::string extension = path.extension().string();
	return HasImporter(extension);
}
