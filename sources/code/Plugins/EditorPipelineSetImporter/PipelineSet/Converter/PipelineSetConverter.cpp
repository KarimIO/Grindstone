#include <string>
#include <iostream>
#include <fstream>

#ifdef _MSC_VER
#include <Windows.h>
#include <debugapi.h>
#endif

#include <PipelineSet/Converter/PipelineSetConditioner.hpp>
#include "Scanner.hpp"
#include "Parser.hpp"
#include "StateResolver.hpp"
#include "ShaderCompiler.hpp"
#include "Output.hpp"

static void SimpleLog(
	Grindstone::LogSeverity severity,
	PipelineConverterLogSource source,
	std::string_view msg,
	const std::filesystem::path& filename,
	uint32_t line,
	uint32_t column
) {
	#ifdef _MSC_VER
		constexpr const WORD debugColorOff = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

		constexpr WORD debugColors[] = {
			FOREGROUND_GREEN,											// Trace: green
			FOREGROUND_GREEN | FOREGROUND_BLUE,							// Info: cyan
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,	// Warning: intense yellow
			FOREGROUND_RED | FOREGROUND_INTENSITY,						// Error: intense red
			BACKGROUND_RED | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY	// Fatal
		};
	#else
		constexpr const char* resetAnsiColor = "\033[0m";
		constexpr const char* ansiColor[] = {
			"\033[1;32m", // Trace: green
			"\033[1;36m", // Info: cyan
			"\033[1;33m", // Warning: yellow
			"\033[1;31m", // Error: red
			"\033[1;35m"  // Fatal: magenta
		};
		const char* ansiColorLogPrefix = ansiColor[static_cast<uint8_t>(level)];
	#endif

	constexpr const char* logPrefixes[] = {
		"[TRACE]",
		"[INFO]",
		"[WARN]",
		"[ERROR]",
		"[FATAL]"
	};

	constexpr const char* logSources[] = {
		"[GENERAL]",
		"[SCANNER]",
		"[PARSER]",
		"[RESOLVER]",
		"[OUTPUT]"
	};

	std::chrono::system_clock::time_point timepoint = std::chrono::system_clock::now();
	time_t coarse = std::chrono::system_clock::to_time_t(timepoint);
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> fine =
		std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint);
	const unsigned long long milliseconds = fine.time_since_epoch().count() % 1000u;

	tm tm;
	localtime_s(&tm, &coarse);

	char timeBuffer[sizeof("23:59:59.999")]{};
	size_t timeOffset = std::strftime(timeBuffer, sizeof timeBuffer - 3, "%T.", &tm);
	std::snprintf(
		timeBuffer + timeOffset,
		4, "%03llu",
		milliseconds
	);

	const char* logPrefix = logPrefixes[static_cast<uint8_t>(severity)];
	const char* logSource = logSources[static_cast<uint8_t>(source)];

	uint8_t index = static_cast<uint8_t>(severity);
	std::string strFilename = filename.string();

	std::string outputPrefix = std::vformat("{}{}[{}] {}", std::make_format_args(logPrefix, logSource, timeBuffer, strFilename));
	if (line != UNDEFINED_LINE) {
		if (column != UNDEFINED_LINE) {
			outputPrefix += std::vformat("({}:{}) : ", std::make_format_args(line, column));
		}
		else {
			outputPrefix += std::vformat("({}) : ", std::make_format_args(line));
		}
	}
	else {
		outputPrefix += " : ";
	}

	#ifdef _MSC_VER
		DWORD outputHandle = severity == Grindstone::LogSeverity::Error
			? STD_ERROR_HANDLE
			: STD_OUTPUT_HANDLE;

		std::string mainMessage = std::vformat("{}\n", std::make_format_args(msg));

		HANDLE hConsole = ::GetStdHandle(outputHandle);
		::SetConsoleTextAttribute(hConsole, debugColors[static_cast<size_t>(severity)]);
		::WriteConsoleA(hConsole, outputPrefix.c_str(), static_cast<DWORD>(outputPrefix.size()), nullptr, nullptr);
		::SetConsoleTextAttribute(hConsole, debugColorOff);

		::WriteConsoleA(hConsole, mainMessage.c_str(), static_cast<DWORD>(mainMessage.size()), nullptr, nullptr);

		::OutputDebugStringA(outputPrefix.c_str());
		::OutputDebugStringA(mainMessage.c_str());
	#else
		std::ostream& consoleStream = (level == LogLevel::Error || level == LogLevel::Fatal)
			? std::cerr
			: std::cout;

		std::string formatted = std::vformat("{}{}{}{}\n");
		consoleStream << formatted;
	#endif
}

static bool TestStringAgainstWildcardPattern(
	std::string_view filename, std::string_view pattern,
	size_t filenameLength, size_t patternLength
) {
	if (patternLength == 0) {
		return true;
	}

	if (filenameLength == 0) {
		for (size_t i = 0; i < patternLength; i++) {
			if (pattern[i] != '*') {
				return false;
			}
		}

		return true;
	}

	if (filename[filenameLength - 1] == pattern[patternLength - 1] || pattern[patternLength - 1] == '?') {
		return TestStringAgainstWildcardPattern(filename, pattern, filenameLength - 1, patternLength - 1);
	}

	if (pattern[patternLength - 1] == '*') {
		return	TestStringAgainstWildcardPattern(filename, pattern, filenameLength, patternLength - 1) ||
			TestStringAgainstWildcardPattern(filename, pattern, filenameLength - 1, patternLength);
	}

	return false;
}

static bool PreprocessFile(LogCallback logCallback, ParseTree& parseTree, const std::filesystem::path& path, std::set<std::filesystem::path>& unprocessedFiles) {
	std::string pathAsStr = path.string();
	if (!std::filesystem::exists(path)) {
		std::string msg = std::vformat("Can't find file: {}", std::make_format_args(pathAsStr));
		logCallback(Grindstone::LogSeverity::Error, PipelineConverterLogSource::General, msg, path, UNDEFINED_LINE, UNDEFINED_COLUMN);
		return false;
	}

	{
		std::string msg = std::vformat("Processing {}", std::make_format_args(pathAsStr));
		logCallback(Grindstone::LogSeverity::Trace, PipelineConverterLogSource::General, msg, path, UNDEFINED_LINE, UNDEFINED_COLUMN);
	}

	std::ifstream ifs(path);
	std::string content(
		(std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>())
	);

	TokenList scanTokens;

	if (!ScanPipelineSet(logCallback, path, content, scanTokens)) {
		std::string msg = std::vformat("Found errors in ScanPipelineState: {}", std::make_format_args(pathAsStr));
		logCallback(Grindstone::LogSeverity::Error, PipelineConverterLogSource::General, msg, path, UNDEFINED_LINE, UNDEFINED_COLUMN);
		return false;
	}

	if (!ParsePipelineSet(logCallback, path, scanTokens, parseTree, unprocessedFiles)) {
		std::string msg = std::vformat("Found errors in ParsePipelineState: {}", std::make_format_args(pathAsStr));
		logCallback(Grindstone::LogSeverity::Error, PipelineConverterLogSource::General, msg, path, UNDEFINED_LINE, UNDEFINED_COLUMN);
		return false;
	}

	return true;
}

static std::filesystem::path SimpleResolve(const std::filesystem::path& path) {
	return path;
}

PipelineSetConditioner::PipelineSetConditioner(WriteCallback writeFn, LogCallback logFn, ResolvePathCallback resolvePathFn)
	: writeFileCallback(writeFn), logCallback(logFn == nullptr ? SimpleLog : logFn), resolvePathCallback(resolvePathFn == nullptr ? SimpleResolve : resolvePathFn) {}

void PipelineSetConditioner::Add(const std::filesystem::path& path) {
	unprocessedFiles.insert(path);
}

void PipelineSetConditioner::Scan(const std::filesystem::path& path) {
	const std::filesystem::path srcFolder = path.parent_path();
	std::string pattern = path.filename().string();

	std::filesystem::recursive_directory_iterator start{ srcFolder };
	std::filesystem::recursive_directory_iterator end;

	for (std::filesystem::recursive_directory_iterator iterator{ start }; iterator != end; ++iterator) {
		if (iterator->is_directory()) {
			continue;
		}

		std::filesystem::path srcPath = iterator->path();
		std::string srcFilename = srcPath.filename().string();
		if (TestStringAgainstWildcardPattern(srcFilename, pattern, srcFilename.size(), pattern.size())) {
			unprocessedFiles.insert(srcPath);
		}
	}
}

void PipelineSetConditioner::Convert(CompilationOptions options) {
	for (const std::filesystem::path& mainFilePath : unprocessedFiles) {
		ParseTree parseTree;
		ResolvedStateTree resolvedStateTree;
		std::set<std::filesystem::path> dependenciesToProcess;
		std::set<std::filesystem::path> resolvedFiles;
		dependenciesToProcess.insert(mainFilePath);

		while (!dependenciesToProcess.empty()) {
			const std::filesystem::path& path = *dependenciesToProcess.begin();
			const std::filesystem::path resolvedPath = resolvePathCallback(path);

			bool hasProcessedThisFile = resolvedFiles.find(resolvedPath) != resolvedFiles.end();
			if (!hasProcessedThisFile) {
				PreprocessFile(logCallback, parseTree, resolvedPath, dependenciesToProcess);
				resolvedFiles.insert(resolvedPath);
			}

			dependenciesToProcess.erase(path);
		}

		ResolveStateTree(logCallback, parseTree, resolvedStateTree);

		std::vector<PipelineOutput> outputFiles;

		for (ResolvedStateTree::PipelineSet& pipelineSet : resolvedStateTree.pipelineSets) {
			CompilationArtifactsGraphics compilationArtifacts;
			if (CompileShadersGraphics(logCallback, pipelineSet, options, compilationArtifacts)) {
				PipelineOutput outputFile;
				if (OutputPipelineSet(logCallback, compilationArtifacts, pipelineSet, outputFile)) {
					outputFiles.emplace_back(outputFile);
				}
			}
		}

		for (ResolvedStateTree::ComputeSet& computeSet : resolvedStateTree.computeSets) {
			CompilationArtifactsCompute compilationArtifacts;
			if (CompileShadersCompute(logCallback, computeSet, options, compilationArtifacts)) {
				PipelineOutput outputFile;
				if (OutputComputeSet(logCallback, compilationArtifacts, computeSet, outputFile)) {
					outputFiles.emplace_back(outputFile);
				}
			}
		}

		writeFileCallback(mainFilePath, outputFiles);
	}

	unprocessedFiles.clear();
}

void PipelineSetConditioner::Rerun(const std::filesystem::path& path) {
	std::string strPath = path.string();
	std::string msg = std::vformat("Rerunning {}", std::make_format_args(strPath));
	logCallback(Grindstone::LogSeverity::Trace, PipelineConverterLogSource::General, msg, path, UNDEFINED_LINE, UNDEFINED_COLUMN);
}

LogCallback PipelineSetConditioner::GetLogCallback() const {
	return logCallback;
}
