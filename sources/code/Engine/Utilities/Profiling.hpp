#ifndef _PROFILING_HPP
#define _PROFILING_HPP

namespace Grindstone {
	namespace Profiler {
		struct Result {
			std::string Name;
			long long Start, End;
			uint32_t ThreadID;
		};

		struct InstrumentationSession {
			std::string Name;
		};

		class Manager {
		private:
			InstrumentationSession* current_session_;
			std::string filepath_;
			std::ofstream output_stream_;
			int profile_count_;
		public:
			Manager() : current_session_(nullptr), profile_count_(0) {}

			void beginSession(const std::string& name, const std::string& filepath = "results.json") {
				filepath_ = filepath;
				output_stream_.open(filepath);
				writeHeader();
				current_session_ = new InstrumentationSession{ name };
				profile_count_ = 0;
			}

			void endSession() {
				writeFooter();
				output_stream_.close();
				GRIND_LOG("Finished writing profile session: {0}", filepath_);
				delete current_session_;
				current_session_ = nullptr;
				profile_count_ = 0;
			}

			void writeProfile(const Result& result) {
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

			void writeHeader() {
				output_stream_ << "{\"otherData\": {},\"traceEvents\":[";
				output_stream_.flush();
			}

			void writeFooter() {
				output_stream_ << "]}";
				output_stream_.flush();
			}

			static Manager& get() {
				static Manager instance;
				return instance;
			}
		};

		class Timer {
		public:
			Timer(const char *name) : name_(name), stopped_(false) {
				start_time_ = std::chrono::high_resolution_clock::now();
			}
			~Timer() {
				if (!stopped_)
					stop();
			}
			void stop() {
				auto end_time = std::chrono::high_resolution_clock::now();

				long long start = std::chrono::time_point_cast<std::chrono::microseconds>(start_time_).time_since_epoch().count();
				long long end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time).time_since_epoch().count();

				uint32_t threadID = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());
				Manager::get().writeProfile({ name_, start, end, threadID });

				stopped_ = true;
			}
		private:
			const char* name_;
			std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
			bool stopped_;
		};
	}
}

#ifdef GRIND_PROFILE
	#define GRIND_PROFILE_BEGIN_SESSION(name, filepath) Grindstone::Profiler::Manager::get().beginSession(name, filepath)
	#define GRIND_PROFILE_END_SESSION() Grindstone::Profiler::Manager::get().endSession()
	#define GRIND_PROFILE_SCOPE(name) Grindstone::Profiler::Timer grindstone_profiler_timer_##__LINE__(name)
	#define GRIND_PROFILE_FUNC() GRIND_PROFILE_SCOPE(__FUNCSIG__)
#else
	#define GRIND_PROFILE_BEGIN_SESSION(name, filepath)
	#define GRIND_PROFILE_END_SESSION()
	#define GRIND_PROFILE_SCOPE(name)
	#define GRIND_PROFILE_FUNC()
#endif

#endif