#pragma once

#include <string>

namespace Grindstone {
	enum class LogSeverity : uint8_t {
		Info,
		Trace,
		Warning,
		Error
	};

	struct ConsoleMessage {
		std::string message;
		LogSeverity logSeverity;
	};
};
