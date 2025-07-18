#pragma once

#include <filesystem>

#include <Common/Editor/Importer.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>
#include <Editor/AssetRegistry.hpp>

#include "Uuid.hpp"

namespace Grindstone::Editor {
	class MetaFile {
	public:
		struct Subasset {
			std::string displayName;
			std::string subassetIdentifier;
			std::string address;
			AssetType assetType = AssetType::Undefined;
			Uuid uuid;

			Subasset() = default;
			Subasset(
				std::string_view subassetIdentifier,
				std::string_view displayName,
				std::string_view address,
				Uuid uuid,
				AssetType type
			) : subassetIdentifier(subassetIdentifier),
				displayName(displayName),
				address(address),
				uuid(uuid),
				assetType(type) {}
		};

	public:
		MetaFile() = default;
		MetaFile(AssetRegistry& assetRegistry, const std::filesystem::path&);

		void Load(AssetRegistry& assetRegistry, const std::filesystem::path&);
		void Save(uint32_t currentImporterVersion);
		void SaveWithoutImporterVersionChange();
		bool TryGetDefaultSubasset(Subasset& subasset) const;
		Uuid GetOrCreateDefaultSubassetUuid(const std::string& subassetName, AssetType assetType);
		Uuid GetOrCreateSubassetUuid(const std::string& subassetName, AssetType assetType);
		size_t GetSubassetCount() const;
		bool TryGetSubasset(const std::string& subassetName, Subasset*& outSubasset);
		bool TryGetSubasset(Uuid uuid, Subasset*& outSubasset);
		bool TryGetDefaultSubassetUuid(Uuid& outUuid) const;
		bool TryGetSubassetUuid(const std::string& subassetName, Uuid& outUuid) const;
		bool IsValid() const;
		bool IsOutdatedImporterVersion(Grindstone::Editor::ImporterVersion currentImporterVersion) const;
		bool IsOutdatedMetaVersion() const;
	public:
		using Iterator = std::vector<Subasset>::iterator;
		using ConstIterator = std::vector<Subasset>::const_iterator;

		Iterator begin() noexcept;
		ConstIterator begin() const noexcept;

		Iterator end() noexcept;
		ConstIterator end() const noexcept;
	private:
		std::string MakeDefaultAddress(std::string_view subassetName) const;

		bool isValid = true;
		uint32_t importerVersion = 0;
		uint32_t metaVersion = 0;
		AssetRegistry& assetRegistry;
		Subasset defaultSubasset;
		std::vector<Subasset> subassets;
		std::filesystem::path metaFilePath;
		std::filesystem::path baseAssetPath;
		std::filesystem::path mountedAssetPath;
	};
}
