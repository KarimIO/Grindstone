#pragma once

#include <string>
#include <chrono>
#include <stdint.h>

namespace Grindstone {
	enum class LogSource : uint16_t {
		Unknown,
		EngineCore,
		Editor,
		EditorImporter,
		GraphicsAPI,
		RenderingBackend,
		Rendering,
		Audio,
		Physics,
		Scripting,
		Count
	};

	constexpr const char* logSourceStrings[] = {
		"Unknown",
		"Engine Core",
		"Editor",
		"Editor Importer",
		"Graphics API",
		"Rendering Backend",
		"Rendering",
		"Audio",
		"Physics",
		"Scripting"
	};

	using LogInternalType = uint32_t;
	constexpr LogInternalType LOG_UNSPECIFIED_INTERNAL_TYPE = UINT32_MAX;
	
	enum class LogSeverity : uint8_t {
		Info,
		Trace,
		Warning,
		Error,
		Fatal
	};

	constexpr const char* logSeverityPrefixes[] = {
		"Info",
		"Trace",
		"Warning",
		"Error",
		"Fatal Error"
	};

	struct ConsoleMessage {
		std::string message;
		std::string filename;
		uint32_t line;
		std::chrono::system_clock::time_point timepoint;
		LogSource source;
		LogInternalType internalType;
		LogSeverity severity;
	};
};
