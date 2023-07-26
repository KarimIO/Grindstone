#pragma once

#include <vector>
#include <map>
#include <filesystem>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>

namespace Grindstone::Editor {
	class AssetRegistry {
	public:
		struct Entry {
			Uuid uuid;
			std::string name;
			std::filesystem::path path;
			AssetType assetType;
		};
	public:
		void Initialize(const std::filesystem::path& projectPath);
		void Cleanup();
		void UpdateEntry(std::filesystem::path& path, std::string& name, Uuid& uuid, AssetType assetType);
		void WriteFile();
		void ReadFile();

		bool HasAsset(Uuid uuid);
		bool TryGetAssetData(Uuid uuid, AssetRegistry::Entry& outEntry);

		void FindAllFilesOfType(AssetType assetType, std::vector<Entry>& outEntries);
	private:
		std::map<Uuid, Entry> assets;
		std::filesystem::path assetsPath;
		std::filesystem::path assetRegistryPath;
	};
}
