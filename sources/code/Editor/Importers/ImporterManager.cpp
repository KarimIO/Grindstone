#include "ImporterManager.hpp"
#include "ModelImporter.hpp"
#include "ShaderImporter.hpp"
#include "TextureImporter.hpp"
#include "AudioImporter.hpp"
#include "MaterialImporter.hpp"
using namespace Grindstone::Importers;

ImporterManager::ImporterManager() {
	AddImporterFactory("fbx", ImportModel);
	AddImporterFactory("dae", ImportModel);
	AddImporterFactory("obj", ImportModel);

	AddImporterFactory("jpeg",ImportTexture);
	AddImporterFactory("jpg", ImportTexture);
	AddImporterFactory("png", ImportTexture);
	AddImporterFactory("tga", ImportTexture);
	AddImporterFactory("bmp", ImportTexture);
	AddImporterFactory("psd", ImportTexture);

	AddImporterFactory("gmat", ImportMaterial);

	AddImporterFactory("glsl", ImportShadersFromGlsl);

	AddImporterFactory("wav", ImportAudio);
}

bool ImporterManager::Import(std::filesystem::path& path) {
	ImporterFactory importerFactory = GetImporterFactoryByPath(path);
	if (importerFactory == nullptr) {
		return false;
	}

	importerFactory(path);
	return true;
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
	std::string extension = path.extension().string().substr(1);
	return GetImporterFactoryByExtension(extension);
}

bool ImporterManager::HasImporter(std::string& extension) {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	return extensionIterator != extensionsToImporterFactories.end();
}

bool ImporterManager::HasImporter(std::filesystem::path& path) {
	std::string extension = path.extension().string().substr(1);
	return HasImporter(extension);
}
