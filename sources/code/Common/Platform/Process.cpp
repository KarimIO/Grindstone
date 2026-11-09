#include <string>
#include <thread>

#include <Windows.h>

#include <Common/Logger.hpp>
#include <Common/Logging.hpp>
#include <EngineCore/Utils/Utilities.hpp>

#include "Process.hpp"

static void PrintInferredSeverity(const std::string& line) {
	std::string lineLower = Grindstone::Utils::ToLower(line);
	Grindstone::LogSeverity severity = Grindstone::LogSeverity::Info;
	if (lineLower.find("error") != std::string::npos) {
		severity = Grindstone::LogSeverity::Error;
	}
	else if (lineLower.find("warn") != std::string::npos) {
		severity = Grindstone::LogSeverity::Warning;
	}

	GPRINT(severity, Grindstone::LogSource::Editor, line.c_str());
}

static void ReadPipeLoop(HANDLE pipe, Grindstone::LogSeverity severity) {
	DWORD bytesRead;
	char buffer[4096]{ 0 };
	std::string partial;
	while (ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
		partial.append(buffer, bytesRead);
		size_t pos;
		while ((pos = partial.find('\n')) != std::string::npos) {
			std::string line = partial.substr(0, pos);
			PrintInferredSeverity(line);
			partial.erase(0, pos + 1);
		}
	}
	if (!partial.empty()) {
		PrintInferredSeverity(partial);  // final line without newline
	}
}

bool Grindstone::Platform::RunCommand(Grindstone::StringRef commandLine, Grindstone::StringRef workingDirectory) {
	SECURITY_ATTRIBUTES securityAttribs{
		.nLength = sizeof(securityAttribs),
		.lpSecurityDescriptor = NULL,
		.bInheritHandle = TRUE
	};

	HANDLE stdoutRead, stdoutWrite, stderrRead, stderrWrite;
	CreatePipe(&stdoutRead, &stdoutWrite, &securityAttribs, 0);
	SetHandleInformation(stdoutRead, HANDLE_FLAG_INHERIT, 0);
	CreatePipe(&stderrRead, &stderrWrite, &securityAttribs, 0);
	SetHandleInformation(stderrRead, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOA si{
		.cb = sizeof(si),
		.dwFlags = STARTF_USESTDHANDLES,
		.hStdInput = NULL,
		.hStdOutput = stdoutWrite,
		.hStdError = stderrWrite,
	};

	Grindstone::String commandLineStr = std::string(commandLine);

	PROCESS_INFORMATION pi{};
	BOOL success = CreateProcessA(
		NULL,
		static_cast<LPSTR>(commandLineStr.data()),
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		workingDirectory.empty() ? nullptr : workingDirectory.data(),
		&si,
		&pi
	);

	CloseHandle(stdoutWrite);
	CloseHandle(stderrWrite);

	if (!success) {
		CloseHandle(stdoutRead);
		CloseHandle(stderrRead);
		return false;
	}

	std::thread readInfoThread(ReadPipeLoop, stdoutRead, Grindstone::LogSeverity::Info);
	std::thread readErrThread(ReadPipeLoop, stderrRead, Grindstone::LogSeverity::Error);

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	readInfoThread.join();
	readErrThread.join();

	return true;
}
