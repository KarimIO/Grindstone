#pragma once

#include <Common/String.hpp>

namespace Grindstone::Platform {
	bool RunCommand(Grindstone::StringRef commandLine, Grindstone::StringRef workingDirectory);
}
