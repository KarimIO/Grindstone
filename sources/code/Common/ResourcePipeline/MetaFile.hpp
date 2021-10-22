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
		Uuid GetOrCreateSubassetUuid(std::string& subassetName);
		bool TryGetSubassetUuid(std::string& subassetName, Uuid& outUuid);
	public:
		struct Subasset {
			std::string name;
			Uuid uuid;

			Subasset(
				std::string name,
				Uuid uuid
			) : name(name), uuid(uuid) {}
		};
	private:
		std::vector<Subasset> subassets;
		std::filesystem::path path;
	};
}
