#pragma once

#include <filesystem>
#include "Common/ResourcePipeline/AssetType.hpp"
#include "Uuid.hpp"

namespace Grindstone {
	class MetaFile {
	public:
		struct Subasset {
			std::string name;
			AssetType assetType = AssetType::Undefined;
			Uuid uuid;

			Subasset() = default;
			Subasset(
				std::string name,
				Uuid uuid,
				AssetType type
			) : name(name), uuid(uuid), assetType(type) {}
		};

	public:
		MetaFile() = default;
		MetaFile(std::filesystem::path);

		void Load(std::filesystem::path);
		void Save();
		bool TryGetDefaultSubasset(Subasset& subasset);
		Uuid GetOrCreateDefaultSubassetUuid(std::string& subassetName, AssetType assetType);
		Uuid GetOrCreateSubassetUuid(std::string& subassetName, AssetType assetType);
		size_t GetSubassetCount();
		bool TryGetDefaultSubassetUuid(Uuid& outUuid);
		bool TryGetSubassetUuid(std::string& subassetName, Uuid& outUuid);
		bool IsOutdatedVersion();
	public:
		using Iterator = std::vector<Subasset>::iterator;
		using ConstIterator = std::vector<Subasset>::const_iterator;

		Iterator begin() noexcept;
		ConstIterator begin() const noexcept;

		Iterator end() noexcept;
		ConstIterator end() const noexcept;
	private:
		uint32_t version = 0;
		Subasset defaultSubasset;
		std::vector<Subasset> subassets;
		std::filesystem::path path;
		std::filesystem::path baseAssetPath;
	};
}
