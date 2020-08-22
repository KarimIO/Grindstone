#include "Profiling.hpp"
#include <thread>
#include <algorithm>

using namespace Grindstone::Profiler;

Manager::Manager() : current_session_(nullptr), profile_count_(0) {}

void Manager::beginSession(const std::string& name, const std::string& filepath) {
	output_stream_.open(filepath);
	writeHeader();
	current_session_ = new InstrumentationSession{ name };
}

void Manager::endSession() {
	writeFooter();
	output_stream_.close();
	delete current_session_;
	current_session_ = nullptr;
	profile_count_ = 0;
}

void Manager::writeProfile(const Result& result) {
	if (profile_count_++ > 0)
		output_stream_ << ",";

	std::string name = result.Name;
	std::replace(name.begin(), name.end(), '"', '\'');

	output_stream_ << "{";
	output_stream_ << "\"cat\":\"function\",";
	output_stream_ << "\"dur\":" << (result.End - result.Start) << ',';
	output_stream_ << "\"name\":\"" << name << "\",";
	output_stream_ << "\"ph\":\"X\",";
	output_stream_ << "\"pid\":0,";
	output_stream_ << "\"tid\":" << result.ThreadID << ",";
	output_stream_ << "\"ts\":" << result.Start;
	output_stream_ << "}";

	output_stream_.flush();
}

void Manager::writeHeader() {
	output_stream_ << "{\"otherData\": {},\"traceEvents\":[";
	output_stream_.flush();
}

void Manager::writeFooter() {
	output_stream_ << "]}";
	output_stream_.flush();
}

Manager& Manager::get() {
	static Manager instance;
	return instance;
}

Timer::Timer(const char *name) : name_(name), stopped_(false) {
	start_time_ = std::chrono::high_resolution_clock::now();
}

Timer::~Timer() {
	if (!stopped_)
		stop();
}

void Timer::stop() {
	auto end_time = std::chrono::high_resolution_clock::now();

	long long start = std::chrono::time_point_cast<std::chrono::microseconds>(start_time_).time_since_epoch().count();
	long long end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time).time_since_epoch().count();

	uint32_t threadID = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());
	Manager::get().writeProfile({ name_, start, end, threadID });

	stopped_ = true;
}
