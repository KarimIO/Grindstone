#pragma once

#include<cstdint>

namespace Grindstone::Platform {
	enum class Configuration : std::uint8_t {
		Auto = 0,
		Debug,
		Release,
		MinSizeRelease,
		ReleaseWithDebugInfo
	};

	const char* ConfigurationToString(Grindstone::Platform::Configuration configuration);
}
