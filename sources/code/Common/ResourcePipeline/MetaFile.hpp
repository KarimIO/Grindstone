#pragma once

#include <filesystem>
#include "Uuid.hpp"

namespace Grindstone {
	class MetaFile {
	public:
		MetaFile() = default;
		MetaFile(std::filesystem::path);

		void Load(std::filesystem::path);
		void Save();
		Uuid GetOrCreateDefaultSubassetUuid(std::string& subassetName);
		Uuid GetOrCreateSubassetUuid(std::string& subassetName);
		bool TryGetDefaultSubassetUuid(Uuid& outUuid);
		bool TryGetSubassetUuid(std::string& subassetName, Uuid& outUuid);
	public:
		struct Subasset {
			std::string name;
			Uuid uuid;

			Subasset() = default;
			Subasset(
				std::string name,
				Uuid uuid
			) : name(name), uuid(uuid) {}
		};

		using Iterator = std::vector<Subasset>::iterator;
		using ConstIterator = std::vector<Subasset>::const_iterator;

		Iterator begin() noexcept;
		ConstIterator begin() const noexcept;

		Iterator end() noexcept;
		ConstIterator end() const noexcept;
	private:
		Subasset defaultSubasset;
		std::vector<Subasset> subassets;
		std::filesystem::path path;
	};
}
