#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone::Editor {
	class MetaFile;

	class AssetRegistry {
	public:
		struct Entry {
			Uuid uuid;
			std::string name;
			std::filesystem::path path;
			AssetType assetType;
		};
	public:
		virtual ~AssetRegistry();

		void Initialize(const std::filesystem::path& projectPath);
		virtual void Cleanup();
		virtual void UpdateEntry(const std::filesystem::path& path, const std::string& name, Uuid& uuid, AssetType assetType);
		virtual void WriteFile();
		virtual void ReadFile();
		virtual Grindstone::Editor::MetaFile* GetMetaFileByPath(const std::filesystem::path& path);
		virtual const std::filesystem::path& GetCompiledAssetsPath();

		virtual bool HasAsset(Uuid uuid) const;
		virtual bool TryGetAssetData(const std::filesystem::path& path, AssetRegistry::Entry& outEntry) const;
		virtual bool TryGetAssetData(Uuid uuid, AssetRegistry::Entry& outEntry) const;

		virtual void FindAllFilesOfType(AssetType assetType, std::vector<Entry>& outEntries) const;
	private:
		std::map<Uuid, Entry> assets;
		std::filesystem::path assetsPath;
		std::filesystem::path compiledAssetsPath;
		std::filesystem::path assetRegistryPath;
	};
}
