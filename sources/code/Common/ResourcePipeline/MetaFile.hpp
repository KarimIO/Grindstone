#pragma once

#include <filesystem>

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
		void Save();
		bool TryGetDefaultSubasset(Subasset& subasset) const;
		Uuid GetOrCreateDefaultSubassetUuid(std::string& subassetName, AssetType assetType);
		Uuid GetOrCreateSubassetUuid(std::string& subassetName, AssetType assetType);
		size_t GetSubassetCount() const;
		bool TryGetSubasset(Uuid uuid, Subasset*& outSubasset);
		bool TryGetDefaultSubassetUuid(Uuid& outUuid) const;
		bool TryGetSubassetUuid(std::string& subassetName, Uuid& outUuid) const;
		bool IsValid() const;
		bool IsOutdatedVersion() const;
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
		uint32_t version = 0;
		AssetRegistry& assetRegistry;
		Subasset defaultSubasset;
		std::vector<Subasset> subassets;
		std::filesystem::path metaFilePath;
		std::filesystem::path baseAssetPath;
		std::filesystem::path mountedAssetPath;
	};
}
