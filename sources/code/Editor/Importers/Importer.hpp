#pragma once

#include <filesystem>

namespace Grindstone {
	namespace Importers {
		class Importer {
			virtual void Import(std::filesystem::path& path) = 0;
		};
	}
}
