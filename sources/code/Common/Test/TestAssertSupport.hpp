#pragma once

#include <gtest/gtest.h>

#include <Common/Assert.hpp>

class AssertEnvironment : public ::testing::Environment {
public:
	void SetUp() override {
		Grindstone::Debug::SetAssertHandler([](const Grindstone::Debug::AssertInfo& info) {
			ADD_FAILURE_AT(info.location.file_name(), static_cast<int>(info.location.line()))
				<< "Unexpected engine assertion: " << info.message;
			return Grindstone::Debug::AssertAction::Continue;
		});
	}
};
// Registers itself, so it works with gtest_main:
static auto* const assertEnvironment = ::testing::AddGlobalTestEnvironment(new AssertEnvironment);

struct AssertFailure : std::runtime_error { using std::runtime_error::runtime_error; };

class ScopedAssertCapture {
public:
	enum class Mode { Record, Throw };

	explicit ScopedAssertCapture(Mode mode = Mode::Record) {
		previous = Grindstone::Debug::SetAssertHandler(
			[this, mode](const Grindstone::Debug::AssertInfo& info) {
				{
					std::lock_guard lock(mutex);
					messages.push_back(info.message);
				}
				if (mode == Mode::Throw) throw AssertFailure(info.message);
				return Grindstone::Debug::AssertAction::Continue;
			});
	}
	~ScopedAssertCapture() { Grindstone::Debug::SetAssertHandler(previous); }

	ScopedAssertCapture(const ScopedAssertCapture&) = delete;
	ScopedAssertCapture& operator=(const ScopedAssertCapture&) = delete;

	size_t Count() const { std::lock_guard lock(mutex); return messages.size(); }
	bool Contains(std::string_view text) const {
		std::lock_guard lock(mutex);
		for (auto& m : messages) if (m.find(text) != std::string::npos) return true;
		return false;
	}

private:
	Grindstone::Debug::AssertHandler previous;
	mutable std::mutex mutex;
	std::vector<std::string> messages;
};
