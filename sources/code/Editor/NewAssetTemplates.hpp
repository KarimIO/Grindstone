#pragma once

#include <string_view>

namespace Grindstone::Editor {
	class AssetTemplateRegistry {
	public:
		void RegisterTemplate(AssetType assetType, std::string_view name, void* sourcePtr, size_t sourceSize);
		void RemoveTemplate(AssetType assetType);
	};
}
