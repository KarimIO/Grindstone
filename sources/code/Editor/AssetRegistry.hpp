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
			std::string displayName;
			std::string subassetIdentifier;
			std::string address;
			std::filesystem::path path;
			AssetType assetType;
		};

	public:
		virtual ~AssetRegistry();

		void Initialize(const std::filesystem::path& projectPath);
		virtual void Cleanup();
		virtual void UpdateEntry(
			const std::filesystem::path& path,
			const std::string_view subassetIdentifier,
			const std::string_view displayName,
			const std::string_view address,
			Uuid& uuid,
			AssetType assetType
		);
		virtual void WriteFile();
		virtual void ReadFile();
		virtual Uuid Import(const std::filesystem::path& path);
		[[nodiscard]] virtual Grindstone::Editor::MetaFile* GetMetaFileByPath(const std::filesystem::path& path);
		[[nodiscard]] virtual const std::filesystem::path& GetCompiledAssetsPath() const;

		virtual bool HasAsset(Uuid uuid) const;
		virtual bool TryGetPathWithMountPoint(const std::filesystem::path& path, std::filesystem::path& outMountedPath) const;
		virtual bool TryGetAssetDataFromAbsolutePath(const std::filesystem::path& path, AssetRegistry::Entry& outEntry) const;
		virtual bool TryGetAssetData(const std::filesystem::path & path, AssetRegistry::Entry & outEntry) const;
		virtual bool TryGetAssetData(const std::string& address, AssetRegistry::Entry& outEntry) const;
		virtual bool TryGetAssetData(Uuid uuid, AssetRegistry::Entry& outEntry) const;

		virtual void FindAllFilesOfType(AssetType assetType, std::vector<Entry>& outEntries) const;
	private:
		std::map<Uuid, Entry> assets;
		std::filesystem::path assetsPath;
		std::filesystem::path compiledAssetsPath;
		std::filesystem::path assetRegistryPath;
	};
}
