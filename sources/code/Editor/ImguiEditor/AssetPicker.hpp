#pragma once

#include <functional>
#include <string>

#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <Editor/AssetRegistry.hpp>

namespace Grindstone::Editor::ImguiEditor {
	class AssetPicker {
	public:
		using AssetPickerCallback = std::function<void(Uuid, std::string)>;

		void OpenPrompt(AssetType assetType, AssetPickerCallback callback);
		void Render();
	private:
		bool isShowing = false;
		AssetPickerCallback callback;
		std::vector<AssetRegistry::Entry> assets;
		std::vector<AssetRegistry::Entry> filteredResults;
		std::string searchString;
	};
}
