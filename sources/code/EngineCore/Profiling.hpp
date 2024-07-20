#ifndef _PROFILING_HPP
#define _PROFILING_HPP

#include <string>
#include <chrono>
#include <vector>
#include <filesystem>

namespace Grindstone {
	namespace Profiler {
		struct Result {
			std::string name;
			float start, end;
			uint32_t depth;
			uint32_t threadId;
		};

		struct InstrumentationSession {
			std::string name;
			std::filesystem::path path;
			std::vector<Result> results;
		};

		class Manager {
		private:
			InstrumentationSession* currentSession = nullptr;
			InstrumentationSession* otherSession = nullptr;
			InstrumentationSession sessionA;
			InstrumentationSession sessionB;
		public:
			Manager();
			void BeginSession(const std::string& name, const std::filesystem::path filepath = "results.json");
			void EndSession();
			void AddProfile(const Result& result);
			static Manager& Get();
			virtual const InstrumentationSession& GetAvailableSession() const;

			std::chrono::system_clock::time_point frameStart;
			bool isInSession = false;
			uint32_t depth;
		};

		class Timer {
		public:
			Timer(const char* name);
			~Timer();
			void Stop();
		private:
			const char* name;
			std::chrono::system_clock::time_point startTime;
			bool stopped;
		};
	}
}

#ifdef _DEBUG
	#define GRIND_PROFILE_BEGIN_SESSION(name, filepath) Grindstone::Profiler::Manager::Get().BeginSession(name, filepath)
	#define GRIND_PROFILE_END_SESSION() Grindstone::Profiler::Manager::Get().EndSession()
	#define GRIND_PROFILE_SCOPE(name) Grindstone::Profiler::Timer grindstone_profiler_timer_##__LINE__(name)
	#define GRIND_PROFILE_FUNC() GRIND_PROFILE_SCOPE(__FUNCSIG__)
#else
	#define GRIND_PROFILE_BEGIN_SESSION(name, filepath)
	#define GRIND_PROFILE_END_SESSION()
	#define GRIND_PROFILE_SCOPE(name)
	#define GRIND_PROFILE_FUNC()
#endif

#endif
