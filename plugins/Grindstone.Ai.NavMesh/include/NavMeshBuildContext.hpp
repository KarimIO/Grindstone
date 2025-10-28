#pragma once

#include <stdint.h>

#include <Recast.h>

namespace Grindstone::Ai {
	// A custom class to override Recast functionality, and redirect functions to integrate with
	// Grindstone functionality, especially logs.
	class GrindstoneRecastContext : public rcContext {
	public:

	protected:
		virtual void doResetLog() override;
		virtual void doLog(const rcLogCategory category, const char* msg, const int len) override;
		virtual void doResetTimers() override;
		virtual void doStartTimer(const rcTimerLabel label) override;
		virtual void doStopTimer(const rcTimerLabel label) override;
		virtual int doGetAccumulatedTime(const rcTimerLabel label) const override;
	};
}
