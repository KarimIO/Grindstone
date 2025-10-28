#include <Grindstone.Ai.NavMesh/include/NavMeshBuildContext.hpp>
#include <EngineCore/Logger.hpp>

void Grindstone::Ai::GrindstoneRecastContext::doResetLog() {}

void Grindstone::Ai::GrindstoneRecastContext::doLog(const rcLogCategory category, const char* msg, const int len) {
	switch (category) {
	case rcLogCategory::RC_LOG_PROGRESS:
		GPRINT_INFO(Grindstone::LogSource::EngineCore, msg);
		break;
	case rcLogCategory::RC_LOG_WARNING:
		GPRINT_WARN(Grindstone::LogSource::EngineCore, msg);
		break;
	case rcLogCategory::RC_LOG_ERROR:
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, msg);
		break;
	}
}

void Grindstone::Ai::GrindstoneRecastContext::doResetTimers() {}
void Grindstone::Ai::GrindstoneRecastContext::doStartTimer(const rcTimerLabel label) {}
void Grindstone::Ai::GrindstoneRecastContext::doStopTimer(const rcTimerLabel label) {}
int Grindstone::Ai::GrindstoneRecastContext::doGetAccumulatedTime(const rcTimerLabel label) const { return 0; }
