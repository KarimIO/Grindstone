#pragma once

#include <string>
#include "Metadata.hpp"

namespace Grindstone {
	namespace Reflection {
		std::string parseDisplayName(std::string v, std::string n);
		std::string parseStoredName(std::string v, std::string n);
		std::string stringifyMetadata(Metadata m);
	}
}
