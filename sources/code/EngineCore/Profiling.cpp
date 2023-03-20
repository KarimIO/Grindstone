#include "Profiling.hpp"
#include <thread>
#include <algorithm>

using namespace Grindstone::Profiler;

Manager::Manager() : currentSession(nullptr), profileCount(0) {}

void Manager::BeginSession(const std::string& name, const std::filesystem::path filepath) {
	std::filesystem::create_directories(filepath.parent_path());
	outputStream.open(filepath);
	WriteHeader();
	currentSession = new InstrumentationSession{ name };
	profileCount = 0;
}

void Manager::EndSession() {
	WriteFooter();
	outputStream.close();
	delete currentSession;
	currentSession = nullptr;
	profileCount = 0;
}

void Manager::WriteProfile(const Result& result) {
	if (profileCount++ > 0)
		outputStream << ",";

	std::string name = result.name;
	std::replace(name.begin(), name.end(), '"', '\'');

	long long duration = result.end - result.start;

	outputStream << "{\"cat\":\"function\",\"dur\":" << duration
		<< ",\"name\":\"" << name << "\",\"ph\":\"X\",\"pid\":0,\"tid\":" << result.threadId
		<< ",\"ts\":" << result.start << "}";

	outputStream.flush();
}

void Manager::WriteHeader() {
	outputStream << "{\"otherData\":{},\"traceEvents\":[";
	outputStream.flush();
}

void Manager::WriteFooter() {
	outputStream << "]}";
	outputStream.flush();
}

Manager& Manager::Get() {
	static Manager instance;
	return instance;
}

Timer::Timer(const char *name) : name(name), stopped(false) {
	startTime = std::chrono::high_resolution_clock::now();
}

Timer::~Timer() {
	if (!stopped)
		Stop();
}

void Timer::Stop() {
	auto end_time = std::chrono::high_resolution_clock::now();

	long long start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
	long long end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time).time_since_epoch().count();

	uint32_t threadID = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());
	Manager::Get().WriteProfile({ name, start, end, threadID });

	stopped = true;
}
