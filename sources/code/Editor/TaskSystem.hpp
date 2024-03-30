#pragma once

#include <map>
#include <mutex>
#include <thread>
#include <functional>
#include <string>

#include <Common/ResourcePipeline/Uuid.hpp>

namespace Grindstone::Editor {
	struct Task {
		enum class Status {
			Queued,
			InProgress,
			Done
		};

		std::string name;
		std::function<void()> fnPtr;
		std::thread thread;
		Status status;

		Task() = default;
		Task(const Task& other);
		Task(Task&& other) = default;
		Task(std::string& name, std::function<void()> fnPtr) : name(name), fnPtr(fnPtr), status(Status::Queued) {}
	};

	class TaskSystem {
	public:
		TaskSystem();
		~TaskSystem();
		std::vector<Task> GetTasks();
		void Execute(std::string jobName, std::function<void()> jobPtr);
		void CullDoneTasks();
		bool HasRunningTasks();
	private:
		std::map<Uuid, Task> tasks;
		std::mutex mutex;
	};
}
