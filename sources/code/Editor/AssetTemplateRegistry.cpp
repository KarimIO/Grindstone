#include "AssetTemplateRegistry.hpp"

using namespace Grindstone::Editor;

AssetTemplateRegistry::AssetTemplate::AssetTemplate(
	AssetType assetType, std::string_view name, std::string_view extension, const void* const sourcePtr, size_t sourceSize)
	: assetType(assetType), name(name), extension(extension) {
	content.resize(sourceSize);
	memcpy(content.data(), sourcePtr, sourceSize);
}

void AssetTemplateRegistry::RegisterTemplate(AssetType assetType, std::string_view name, std::string_view extension, const void* const sourcePtr, size_t sourceSize) {
	assetTemplates.emplace_back(assetType, name, extension, sourcePtr, sourceSize);
}

void AssetTemplateRegistry::RemoveTemplate(AssetType assetType) {
	size_t assetTypeIndex = SIZE_MAX;
	for (size_t index = 0; index < assetTemplates.size(); ++index) {
		if (assetTemplates[index].assetType == assetType) {
			assetTypeIndex = index;
			break;
		}
	}

	if (assetTypeIndex == SIZE_MAX) {
		return;
	}

	assetTemplates.erase(assetTemplates.begin() + assetTypeIndex);
}

AssetTemplateRegistry::TemplateList::iterator AssetTemplateRegistry::begin() {
	return assetTemplates.begin();
}

AssetTemplateRegistry::TemplateList::const_iterator AssetTemplateRegistry::begin() const {
	return assetTemplates.begin();
}

AssetTemplateRegistry::TemplateList::iterator AssetTemplateRegistry::end() {
	return assetTemplates.end();
}

AssetTemplateRegistry::TemplateList::const_iterator AssetTemplateRegistry::end() const {
	return assetTemplates.end();
}
