#pragma once

#include <string>
#include "Metadata.hpp"

namespace Grindstone {
	namespace Reflection {
		std::string ParseDisplayName(std::string v, std::string n);
		std::string ParseStoredName(std::string v, std::string n);
		std::string StringifyMetadata(Metadata m);
	}
}
