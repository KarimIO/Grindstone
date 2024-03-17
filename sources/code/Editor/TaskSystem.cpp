#include <Windows.h>

#include "TaskSystem.hpp"

using namespace Grindstone::Editor;


Task::Task(const Task& other)
	: fnPtr(other.fnPtr), name(other.name), status(other.status), thread() {}

TaskSystem::TaskSystem() {
}

TaskSystem::~TaskSystem() {}

std::vector<Task> TaskSystem::GetTasks() {
	std::scoped_lock lock(mutex);

	std::vector<Task> outTasks;
	outTasks.reserve(tasks.size());

	for (auto& task : tasks) {
		outTasks.emplace_back(task.second);
	}

	return outTasks;
}

void TaskSystem::Execute(std::string jobName, std::function<void()> jobPtr) {
	Uuid uuid = Uuid::CreateRandom();
	{
		std::scoped_lock lock(mutex);
		Task& task = tasks[uuid];
		task.name = jobName;
		task.fnPtr = jobPtr;
		task.status = Task::Status::InProgress;
	}

	//tasks[uuid].thread = std::thread([jobPtr, uuid, this] {
		try {
			jobPtr();
		}
		catch (std::runtime_error e) {
			OutputDebugString(e.what());
		}
		std::scoped_lock lock(mutex);
		tasks[uuid].status = Task::Status::Done;
	//});
}
