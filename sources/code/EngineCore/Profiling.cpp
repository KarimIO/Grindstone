#include <algorithm>
#include <fstream>
#include <thread>

#include "Profiling.hpp"

using namespace Grindstone::Profiler;

static void WriteProfile(std::ofstream& outputStream, const Result& result) {
	std::string name = result.name;
	std::replace(name.begin(), name.end(), '"', '\'');

	long long duration = static_cast<long long>(result.end - result.start);

	outputStream << "{\"cat\":\"function\",\"dur\":" << duration
		<< ",\"name\":\"" << name << "\",\"ph\":\"X\",\"pid\":0,\"tid\":" << result.threadId
		<< ",\"ts\":" << result.start << "}";
}

Manager::Manager() : currentSession(&sessionA), otherSession(&sessionB), depth(0) {
	sessionA.results.reserve(1000);
	sessionB.results.reserve(1000);
}

void Manager::BeginSession(const std::string& name, const std::filesystem::path filepath) {
	if (currentSession == &sessionB) {
		currentSession = &sessionA;
		otherSession = &sessionB;
	}
	else {
		currentSession = &sessionB;
		otherSession = &sessionA;
	}

	currentSession->name = name;
	currentSession->path = filepath;
	currentSession->results.clear();

	frameStart = std::chrono::system_clock::now();
	isInSession = true;
}

void Manager::EndSession() {
	if (currentSession == nullptr) {
		return;
	}

	std::ofstream outputStream;
	std::filesystem::create_directories(currentSession->path.parent_path());
	outputStream.open(currentSession->path);

	size_t resultCount = currentSession->results.size();

	outputStream << "{\"otherData\":{},\"traceEvents\":[";

	if (resultCount > 0) {
		WriteProfile(outputStream, currentSession->results[0]);
	}

	for (size_t i = 1; i < resultCount; ++i) {
		outputStream << ",";
		WriteProfile(outputStream, currentSession->results[i]);
	}

	outputStream << "]}";
	outputStream.close();

	isInSession = false;
}

void Manager::AddProfile(const Result& result) {
	if (isInSession) {
		currentSession->results.push_back(result);
	}
}

Manager& Manager::Get() {
	static Manager instance;
	return instance;
}

const InstrumentationSession& Grindstone::Profiler::Manager::GetAvailableSession() const {
	return *otherSession;
}

Timer::Timer(const char *name) : name(name), stopped(false) {
	startTime = std::chrono::system_clock::now();
	Manager& manager = Manager::Get();
	++manager.depth;
}

Timer::~Timer() {
	if (!stopped) {
		Stop();
	}
}

void Timer::Stop() {
	uint32_t threadID = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());
	Manager& manager = Manager::Get();
	std::chrono::system_clock::time_point frameStart = manager.frameStart;

	std::chrono::duration<float> start = startTime - frameStart;
	std::chrono::duration<float> end = std::chrono::system_clock::now() - frameStart;

	manager.AddProfile({ name, start.count(), end.count(), manager.depth, threadID});
	--manager.depth;

	stopped = true;
}
