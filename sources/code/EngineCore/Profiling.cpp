#include "Profiling.hpp"
#include <thread>
#include <algorithm>

using namespace Grindstone::Profiler;

Manager::Manager() : currentSession(nullptr), profileCount(0) {}

void Manager::BeginSession(const std::string& name, const std::string& filepath) {
	outputStream.open(filepath);
	WriteHeader();
	currentSession = new InstrumentationSession{ name };
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

	outputStream << "\t\t{\n";
	outputStream << "\t\t\t\"cat\":\"function\",\n";
	outputStream << "\t\t\t\"dur\":" << duration << ",\n";
	outputStream << "\t\t\t\"name\":\"" << name << "\",\n";
	outputStream << "\t\t\t\"ph\":\"X\",\n";
	outputStream << "\t\t\t\"pid\":0,\n";
	outputStream << "\t\t\t\"tid\":" << result.threadID << ",\n";
	outputStream << "\t\t\t\"ts\":" << result.start << "\n";
	outputStream << "\t\t}\n";

	outputStream.flush();
}

void Manager::WriteHeader() {
	outputStream << "{\n\t\"otherData\": {},\n\t\"traceEvents\":[\n";
	outputStream.flush();
}

void Manager::WriteFooter() {
	outputStream << "\t]\n}\n";
	outputStream.flush();
}

Manager& Manager::Get() {
	static Manager instance;
	return instance;
}

Timer::Timer(const char *name) : name(name), stopped(false) {
	start_time_ = std::chrono::high_resolution_clock::now();
}

Timer::~Timer() {
	if (!stopped)
		Stop();
}

void Timer::Stop() {
	auto end_time = std::chrono::high_resolution_clock::now();

	long long start = std::chrono::time_point_cast<std::chrono::microseconds>(start_time_).time_since_epoch().count();
	long long end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time).time_since_epoch().count();

	uint32_t threadID = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());
	Manager::Get().WriteProfile({ name, start, end, threadID });

	stopped = true;
}
