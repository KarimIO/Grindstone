#ifndef _PROFILING_HPP
#define _PROFILING_HPP

#include <fstream>
#include <string>
#include <chrono>

namespace Grindstone {
	namespace Profiler {
		struct Result {
			std::string name;
			long long start, end;
			uint32_t threadID;
		};

		struct InstrumentationSession {
			std::string name;
		};

		class Manager {
		private:
			InstrumentationSession* currentSession;
			std::string path;
			std::ofstream outputStream;
			int profileCount;
		public:
			Manager();
			void beginSession(const std::string& name, const std::string& filepath = "results.json");
			void endSession();
			void writeProfile(const Result& result);
			void writeHeader();
			void writeFooter();
			static Manager& get();
		};

		class Timer {
		public:
			Timer(const char* name);
			~Timer();
			void stop();
		private:
			const char* name;
			std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
			bool stopped;
		};
	}
}

#ifdef _DEBUG
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