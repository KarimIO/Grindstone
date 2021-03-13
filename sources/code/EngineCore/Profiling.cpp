#include "Profiling.hpp"
#include <thread>
#include <algorithm>

using namespace Grindstone::Profiler;

Manager::Manager() : currentSession(nullptr), profileCount(0) {}

void Manager::beginSession(const std::string& name, const std::string& filepath) {
	outputStream.open(filepath);
	writeHeader();
	currentSession = new InstrumentationSession{ name };
}

void Manager::endSession() {
	writeFooter();
	outputStream.close();
	delete currentSession;
	currentSession = nullptr;
	profileCount = 0;
}

void Manager::writeProfile(const Result& result) {
	if (profileCount++ > 0)
		outputStream << ",";

	std::string name = result.name;
	std::replace(name.begin(), name.end(), '"', '\'');

	outputStream << "\t{";
	outputStream << "\t\t\"cat\":\"function\",";
	outputStream << "\t\t\"dur\":" << (result.end - result.start) << ',';
	outputStream << "\t\t\"name\":\"" << name << "\",";
	outputStream << "\t\t\"ph\":\"X\",";
	outputStream << "\t\t\"pid\":0,";
	outputStream << "\t\t\"tid\":" << result.threadID << ",";
	outputStream << "\t\t\"ts\":" << result.start;
	outputStream << "\t}";

	outputStream.flush();
}

void Manager::writeHeader() {
	outputStream << "{\"otherData\": {},\"traceEvents\":[";
	outputStream.flush();
}

void Manager::writeFooter() {
	outputStream << "]}";
	outputStream.flush();
}

Manager& Manager::get() {
	static Manager instance;
	return instance;
}

Timer::Timer(const char *name) : name(name), stopped(false) {
	start_time_ = std::chrono::high_resolution_clock::now();
}

Timer::~Timer() {
	if (!stopped)
		stop();
}

void Timer::stop() {
	auto end_time = std::chrono::high_resolution_clock::now();

	long long start = std::chrono::time_point_cast<std::chrono::microseconds>(start_time_).time_since_epoch().count();
	long long end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time).time_since_epoch().count();

	uint32_t threadID = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());
	Manager::get().writeProfile({ name, start, end, threadID });

	stopped = true;
}
